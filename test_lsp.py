#!/usr/bin/env python3
import subprocess
import json
import sys

def send_lsp_request(method, params=None, id=1):
    """Send an LSP request and return the response"""
    if params is None:
        params = {}
    
    request = {
        "jsonrpc": "2.0",
        "id": id,
        "method": method,
        "params": params
    }
    
    request_json = json.dumps(request)
    message = f"Content-Length: {len(request_json)}\r\n\r\n{request_json}"
    
    return message

def test_lsp_server():
    """Test the LSP server with basic requests"""
    
    # Start the LSP server
    proc = subprocess.Popen(
        ['./tools/glulsp/sources/glulsp'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    # Send initialize request
    init_message = send_lsp_request("initialize", {
        "processId": 12345,
        "rootPath": "/tmp",
        "capabilities": {}
    })
    
    print("Sending initialize request...")
    print(repr(init_message))
    
    proc.stdin.write(init_message)
    proc.stdin.flush()
    
    # Read response
    response = proc.stdout.readline()
    print(f"Response: {response}")
    
    # Send shutdown
    shutdown_message = send_lsp_request("shutdown", {})
    proc.stdin.write(shutdown_message)
    proc.stdin.flush()
    
    # Send exit
    exit_message = send_lsp_request("exit", {}, id=None)
    proc.stdin.write(exit_message)
    proc.stdin.flush()
    
    proc.wait()
    print("LSP server test completed")

if __name__ == "__main__":
    test_lsp_server()