<h1>valstat for callable constructors</h1><!-- omit in toc -->
 
| &nbsp;           | &nbsp;                                                   |
| ---------------- | -------------------------------------------------------- |
| Document Number: | **PXXXX** |
| Date             | 2021-APR-?? |
| Potential Audience | WG21 |
| Author           | Dusan B. Jovanovic ( [dbj@dbj.org](mailto:dbj@dbj.org) ) |

<h2>&nbsp;</h2>

- [1. Abstract](#1-abstract)
- [2. Motivation](#2-motivation)
- [3. Synopsis](#3-synopsis)
  - [3.1. Rule: C Constructor return type must be nested valstat type of the same class](#31-rule-c-constructor-return-type-must-be-nested-valstat-type-of-the-same-class)
  - [3.2. Rule: there can be one or more nested types following the rule 1](#32-rule-there-can-be-one-or-more-nested-types-following-the-rule-1)
  - [3.3. Rule: C constructors have the same signature as any other non callable constructors.](#33-rule-c-constructors-have-the-same-signature-as-any-other-non-callable-constructors)
  - [3.4. Rule:  Callable constructors can not be declared explicit.](#34-rule--callable-constructors-can-not-be-declared-explicit)
  - [3.5. Rule: compiler generated constructors and assignments are not callable.](#35-rule-compiler-generated-constructors-and-assignments-are-not-callable)
  - [3.6. Rule: Assignments can not be callable in any case](#36-rule-assignments-can-not-be-callable-in-any-case)
  - [3.7. Rule: compiler deleted constructors and assignments are obviously not callable](#37-rule-compiler-deleted-constructors-and-assignments-are-obviously-not-callable)
  - [3.8. Side effect: noexcept ctor is certain at last](#38-side-effect-noexcept-ctor-is-certain-at-last)
  - [3.9. Callable Constructors and Unfinished instances](#39-callable-constructors-and-unfinished-instances)
- [4. The usage](#4-the-usage)
  - [4.1. Legacy](#41-legacy)
  - [4.2. Rule: Compiler ignores returns from Callable Constructors if T is created on the heap.](#42-rule-compiler-ignores-returns-from-callable-constructors-if-t-is-created-on-the-heap)
  - [4.3. valstat returns two step decoding](#43-valstat-returns-two-step-decoding)
  - [4.4. Rule: If constructor does not contain return statements, it can not be called.](#44-rule-if-constructor-does-not-contain-return-statements-it-can-not-be-called)
- [5. Conclusion](#5-conclusion)
- [6. Appendix: The valstat nano course](#6-appendix-the-valstat-nano-course)

<h2>&nbsp;</h2>

"There are two ways of constructing a software design: One way is to make it so simple that there are obviously no deficiencies, and the other way is to make it so complicated that there are no obvious deficiencies. The first method is far more difficult." -- [C.A.R. Hoare](https://en.wikiquote.org/wiki/C._A._R._Hoare)

<h2>&nbsp;</h2>

## 1. Abstract

This paper describes very simple C++callable constructors proposal, by imposing a core language change that is fundamental but in the same time non breaking.

For the actual implementation the [VALSTAT](https://github.com/DBJDBJ/valstat) protocol definition is deployed.

## 2. Motivation

<!-- In the constructor the runtime uses type data to determine how much space is needed to store an object instance in memory. After this space is allocated, the constructor is called as an internal part of the instantiation and initialization process to initialize the contents of the instance.

Then the constructor exits, the runtime returns the newly-created instance. So the reason the constructor doesn't return a value is because it's not called directly, it's called by the memory allocation and object initialization code. -->

Standard C++ constructors only way to signal the outcome, is to throw an exception. Projects where exception are mandated to be non existent are using factory methods to create class instances. That has effectively created another C++ dialect and fragmented the community.

Valstat structure, sometimes called "valstat carrier", is a record made of two fields: value and status. Field can be empty or occupied.

Using valstat carrier as a return type we can navigate a lightweight and simple route around tha inability of constructors to return values. 

Cost of this mechanism is almost zero. 

## 3. Synopsis

**Vocabulary**
| Term          | Meaning                                         |
| ------------- | ----------------------------------------------- |
| C Ctor        |
| C Constructor | Callable Constructor                            |
| CC Class      | Class having one or more Callable Constructors  |
| CC Struct     | Struct having one or more Callable Constructors |

CC Class can have a mixture of callable and non-callable (aka "normal") constructors. 

**Specimen CC Class:**
```cpp
// CC Struct
struct person 
{
struct valstat { person * value; const char * status; };

std::string name ; // data

~person () { if (! name.empty() ) name = ""; }

person person ( string new_name_) : name (new_name_) 
{  
 return valstat{ *this, "person constructed" };
}

person () noexcept : name("") 
{
    return valstat{ *this, "default person constructed" };
}

person ( person const & ) noexcept = delete ;
person & operator = ( person const & ) noexcept = delete ;

person & operator = ( person && other) { 
    using namespace std;
    std::swap(this->name, other.name) ;
    return *this; 
} 

person ( person && another_ ) noexcept : name ( another_.name ) { 
   another_.name = "" ;
}

} ; // eof person
```
**Comments, rules and explanations**
```cpp
struct person 
{
```  
### 3.1. Rule: C Constructor return type must be nested valstat type of the same class
value field type must be the pointer to the type being constructed. 
```cpp
  // T::valstat declaration
  // C Ctor has to return instance of this struct
  struct valstat { person * value; const char * status; };
```
Value of the T::valstat field must no be freed. T::valstat field can point to non existent T instance.

### 3.2. Rule: there can be one or more nested types following the rule 1

Callable `person` constructors must return `person::valstat`.

### 3.3. Rule: C constructors have the same signature as any other non callable constructors. 
All the other language rules for constructors do apply.

If constructor has no return type in its implementation it can not be called; it is a "normal" constructor.

### 3.4. Rule:  Callable constructors can not be declared explicit. 

Callable constructors:

```cpp
    std::string name ; // data

   person () noexcept : name("") 
   {
/* person::valstat in an OK state */
       return valstat{ *this, "default person constructed" };
   }
   
   person person ( string new_name_) : name (new_name_) 
   {
/* person::valstat in an OK state */
      return valstat{ *this, "person constructed" };
   }
```
Constructor return type rules are the same rules as for any other function except that return type is not declared. As ever on standard C++ constructors
```cpp
// destructors were always callable
~person () { if (! empty() ) name = ""; }
```
### 3.5. Rule: compiler generated constructors and assignments are not callable.
### 3.6. Rule: Assignments can not be callable in any case
```cpp
person & operator = ( person const & ) noexcept = default ;
person & operator = ( person && ) noexcept = default ;
```
### 3.7. Rule: compiler deleted constructors and assignments are obviously not callable
Copy or move constructor, signature is unchanged. Declarations of constructors and assignments are same as ever before.
```cpp    
person ( person const & another_ ) noexcept = delete ;
```
### 3.8. Side effect: noexcept ctor is certain at last

Until now noexcept constructors have been a best guess. In this scenario noexcept might be finally a true mark of no exceptions thrown. At least when callable constructors are concerned.

### 3.9. Callable Constructors and Unfinished instances

In case of returning a valstat prematurely i.e.from an unfinished object ctor, `this` will be a nullptr. Compiler should be able to catch that as an error.

## 4. The usage

### 4.1. Legacy

One is free to construct T on heap and do traditional constructions as ever before. Thus the legacy code is unaffected.
```cpp
// legacy instantiation syntax
// using a constructor as ever before
// valstat is not returned
person p() ;
// ignoring the valstat returned
return person() ;
```
Only explicit assignment to T::valstat will provoke callable constructors return to be passed out.  
```cpp
void login ( person p);
// resulting in a person instance
// not person::valstat
login( person() ); 

// Reminder: valstat field = state + data
void check ( person::valstat const & pv ) {
  // CAUTION: value might point to a temporary object
  if ( pv.value )
   logging() << pv.value->name << " checks OK";
}
// constructor return passed as argument value
check( person() ) ;
```
### 4.2. Rule: Compiler ignores returns from Callable Constructors if T is created on the heap.
```cpp
person * pp = new person();
```
Compiler should be able to resolve the above easily.

### 4.3. valstat returns two step decoding

valstat structure carries information. Information = state + data.

One could naturally decode the full valstat information returned as a result of a constructor call.
```cpp
// using the callable constructor 
// return type is: person::valstat
// pp is a person instance pointer
// status type is "const char *"
auto [ pp , status ] = person() ;

// the standard valstat information two steps decoding
if ( pp ) {
    std::cout << "new person is created: " << pp->name ;
} else {
  std::cout << "person default constructor has failed." ;
}

if ( status ) {
    std::cout << "status is: " << status ;
} else
    std::cout << "status is empty" ;
} 

```
### 4.4. Rule: If constructor does not contain return statements, it can not be called.
```cpp
// compilation error -- non callable constructor
auto [ p , status ] = person("Mr Person") ;
```

<h2>&nbsp;</h2>

## 5. Conclusion
Can this mechanism be abused? Anything in C++ can be abused. Standard constructor paradigm can be abused. Callable constructors can be abused too.

This language extension would not break any existing code. 


## 6. Appendix: The valstat nano course

`T::valstat` is a mandated type whose instance is returned from a callable constructor of a type T.

Instances of that type are used to "carry one the four states" as described in [VALSTAT](https://github.com/DBJDBJ/valstat/blob/master/VALSTAT.md).

Combination of value *and* status occupancies is giving four possible states. 

| Meta State Label | Value occupancy | op  | Status occupancy |
| ---------------- | --------------- | --- | ---------------- |
| **Info**         | Has value       | AND | Has value        |
| **OK**           | Has value       | AND | Empty            |
| **Error**        | Empty           | AND | Has value        |
| **Empty**        | Empty           | AND | Empty            |

**Valstat type is the valstat carrier.**

> Information = state + data

*EOF*