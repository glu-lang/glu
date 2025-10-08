use crate::types::{GluFunction, GluType};

pub struct Parser<'a> {
    input: &'a str,
    pub pos: usize,
}

impl<'a> Parser<'a> {
    pub fn new(input: &'a str) -> Self {
        Self { input, pos: 0 }
    }

    /// Parse the entire function
    ///
    /// # Examples
    /// ```
    /// use glu_demangle::parser::Parser;
    /// use glu_demangle::types::{GluFunction, GluType};
    ///
    /// let mut parser = Parser::new("4main4testFvR");
    /// let result = parser.parse_function().unwrap();
    ///
    /// assert_eq!(result.module_path, vec!["main"]);
    /// assert_eq!(result.name, "test");
    /// assert_eq!(result.return_type, GluType::Void);
    /// assert_eq!(result.parameters.len(), 0);
    /// ```
    pub fn parse_function(&mut self) -> anyhow::Result<GluFunction> {
        // Parse all module and function name components first
        let mut components = Vec::new();

        while self.pos < self.input.len() && self.peek()?.is_ascii_digit() {
            components.push(self.parse_string()?);
        }

        if components.is_empty() {
            return Err(anyhow::anyhow!("No function name found"));
        }

        // Last component is the function name, others are module path
        let name = components.pop().unwrap();
        let module_path = components;

        let function_type = self.parse_type()?;

        if let GluType::Function { return_type, params } = function_type {
            Ok(GluFunction {
                module_path,
                name,
                return_type: *return_type,
                parameters: params,
            })
        } else {
            Err(anyhow::anyhow!("Expected function type"))
        }
    }

    /// Parse a length-prefixed string
    ///
    /// # Examples
    /// ```
    /// use glu_demangle::parser::Parser;
    ///
    /// let mut parser = Parser::new("4test");
    /// let result = parser.parse_string().unwrap();
    /// assert_eq!(result, "test");
    /// assert_eq!(parser.pos, 5);
    /// ```
    pub fn parse_string(&mut self) -> anyhow::Result<String> {
        let len = self.parse_number()?;
        let start = self.pos;
        let end = start + len;

        if end > self.input.len() {
            return Err(anyhow::anyhow!("String length exceeds input"));
        }

        let result = self.input[start..end].to_string();
        self.pos = end;
        Ok(result)
    }

    /// Parse a number from the input
    ///
    /// # Examples
    /// ```
    /// use glu_demangle::parser::Parser;
    ///
    /// let mut parser = Parser::new("123abc");
    /// let result = parser.parse_number().unwrap();
    /// assert_eq!(result, 123);
    /// assert_eq!(parser.pos, 3);
    /// ```
    pub fn parse_number(&mut self) -> anyhow::Result<usize> {
        let start = self.pos;
        while self.pos < self.input.len() && self.peek()?.is_ascii_digit() {
            self.pos += 1;
        }

        if start == self.pos {
            return Err(anyhow::anyhow!("Expected number"));
        }

        self.input[start..self.pos].parse()
            .map_err(|_| anyhow::anyhow!("Invalid number"))
    }

    /// Parse a type from the input
    ///
    /// # Examples
    /// ```
    /// use glu_demangle::parser::Parser;
    /// use glu_demangle::types::GluType;
    ///
    /// // Basic types
    /// let mut parser = Parser::new("v");
    /// assert_eq!(parser.parse_type().unwrap(), GluType::Void);
    ///
    /// let mut parser = Parser::new("b");
    /// assert_eq!(parser.parse_type().unwrap(), GluType::Bool);
    ///
    /// let mut parser = Parser::new("i32");
    /// assert_eq!(parser.parse_type().unwrap(), GluType::Int { signed: true, width: 32 });
    ///
    /// // Pointer type
    /// let mut parser = Parser::new("Pi32");
    /// let result = parser.parse_type().unwrap();
    /// if let GluType::Pointer(pointee) = result {
    ///     assert_eq!(*pointee, GluType::Int { signed: true, width: 32 });
    /// }
    ///
    /// // Function type
    /// let mut parser = Parser::new("Fi32i32vR");
    /// let result = parser.parse_type().unwrap();
    /// if let GluType::Function { return_type, params } = result {
    ///     assert_eq!(*return_type, GluType::Int { signed: true, width: 32 });
    ///     assert_eq!(params.len(), 2);
    ///     assert_eq!(params[0], GluType::Int { signed: true, width: 32 });
    ///     assert_eq!(params[1], GluType::Void);
    /// }
    /// ```
    pub fn parse_type(&mut self) -> anyhow::Result<GluType> {
        let c = self.consume()?;
        match c {
            'v' => Ok(GluType::Void),
            'b' => Ok(GluType::Bool),
            'c' => Ok(GluType::Char),
            'N' => Ok(GluType::Null),
            'D' => Ok(GluType::DynamicArray),
            'i' => {
                let width = self.parse_number()? as u32;
                Ok(GluType::Int { signed: true, width })
            }
            'u' => {
                let width = self.parse_number()? as u32;
                Ok(GluType::Int { signed: false, width })
            }
            'f' => {
                let width = self.parse_number()? as u32;
                Ok(GluType::Float { width })
            }
            'P' => {
                let pointee = Box::new(self.parse_type()?);
                Ok(GluType::Pointer(pointee))
            }
            'A' => {
                let size = self.parse_number()?;
                let element_type = Box::new(self.parse_type()?);
                Ok(GluType::StaticArray { size, element_type })
            }
            'F' => {
                let return_type = Box::new(self.parse_type()?);
                let mut params = Vec::new();

                while self.peek()? != 'R' {
                    params.push(self.parse_type()?);
                }
                self.consume()?; // consume 'R'

                Ok(GluType::Function { return_type, params })
            }
            'T' => {
                // Parse module path for struct/enum
                let mut module_path = Vec::new();
                while self.pos < self.input.len() && self.peek()?.is_ascii_digit() {
                    module_path.push(self.parse_string()?);
                }

                // Last component is the name, others are module path
                let name = module_path.pop().unwrap_or_default();

                let kind = self.consume()?;

                match kind {
                    'S' => Ok(GluType::Struct { module_path, name }),
                    'E' => Ok(GluType::Enum { module_path, name }),
                    _ => Err(anyhow::anyhow!("Invalid type kind: {}", kind)),
                }
            }
            _ => Err(anyhow::anyhow!("Unknown type: {}", c)),
        }
    }

    fn peek(&self) -> anyhow::Result<char> {
        self.input.chars().nth(self.pos)
            .ok_or_else(|| anyhow::anyhow!("Unexpected end of input"))
    }

    fn consume(&mut self) -> anyhow::Result<char> {
        let c = self.peek()?;
        self.pos += 1;
        Ok(c)
    }
}
