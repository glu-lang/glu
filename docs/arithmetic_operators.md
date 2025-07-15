# Système d'opérateurs arithmétiques intrinsèques

Ce document décrit comment les opérateurs arithmétiques de base (+, -, *, /) sont implémentés dans le compilateur Glu en tant qu'intrinsèques, évitant ainsi la nécessité d'importer explicitement des fonctions C.

## Architecture

### 1. Niveau AST
- Les opérateurs sont représentés par `BinaryOpExpr` avec l'opérateur stocké comme `RefExpr`
- Les tokens des opérateurs sont définis dans `TokenKind.def`

### 2. Niveau GIL (Glu Intermediate Language)
- **Instructions arithmétiques** : `AddInst`, `SubInst`, `MulInst`, `DivInst` pour les entiers
- **Instructions flottantes** : `FAddInst`, `FSubInst`, `FMulInst`, `FDivInst` pour les flottants
- **Classe de base** : `ArithmeticInst` pour toutes les instructions arithmétiques

### 3. Génération de code
- **GILGen** : Détecte les opérateurs intrinsèques et génère les instructions GIL appropriées
- **IRGen** : Traduit les instructions GIL en instructions LLVM IR

## Implémentation

### Fichiers principaux

1. **`include/GIL/InstKind.def`** : Définition des types d'instructions arithmétiques
2. **`include/GIL/Instructions/ArithmeticInst.hpp`** : Classe de base pour les instructions arithmétiques
3. **`include/GIL/Instructions/AddInst.hpp`** : Implémentations des instructions arithmétiques spécifiques
4. **`lib/GILGen/Intrinsics.hpp`** : Générateur d'intrinsèques pour les opérateurs arithmétiques
5. **`lib/GILGen/GILGenExpr.hpp`** : Génération GIL pour les expressions binaires (modifié)
6. **`lib/IRGen/IRGen.cpp`** : Génération LLVM IR pour les instructions arithmétiques (modifié)

### Flux de compilation

1. **Parsing** : `a + b` → `BinaryOpExpr(RefExpr("+"), a, b)`
2. **GILGen** : Détecte que "+" est un opérateur intrinsèque → génère `AddInst` ou `FAddInst`
3. **IRGen** : Traduit `AddInst` → `llvm::IRBuilder::CreateAdd()`

### Types supportés

- **Entiers** : `Int` (32 bits par défaut)
- **Flottants** : `Float` (32 bits par défaut)  
- **Conversions automatiques** : `Int + Float` → `Float`

## Utilisation

```glu
func main() {
    let a: Int = 10;
    let b: Int = 20;
    let sum = a + b;    // Génère AddInst
    
    let x: Float = 3.14;
    let y: Float = 2.71;
    let fsum = x + y;   // Génère FAddInst
    
    let mixed = a + x;  // Génère FAddInst avec conversion
}
```

## Avantages

1. **Performance** : Pas d'appel de fonction, instructions directes
2. **Optimisation** : LLVM peut optimiser les opérations arithmétiques
3. **Simplicité** : Pas besoin d'importer des fonctions C
4. **Typage** : Vérification de type automatique et conversions

## Extension

Pour ajouter de nouveaux opérateurs intrinsèques :

1. Ajouter l'instruction dans `InstKind.def`
2. Créer la classe d'instruction dans `Instructions/`
3. Ajouter le support dans `Intrinsics.hpp`
4. Implémenter la génération LLVM dans `IRGen.cpp`

## Tests

Les tests sont dans `test/GILGen/ArithmeticIntrinsicsTest.cpp` et couvrent :
- Génération d'instructions pour différents types
- Détection des opérateurs intrinsèques
- Conversion de types automatique
