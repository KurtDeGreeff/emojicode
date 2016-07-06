//
//  PackageParser.cpp
//  Emojicode
//
//  Created by Theo Weidmann on 24/04/16.
//  Copyright © 2016 Theo Weidmann. All rights reserved.
//

#include <cstring>
#include "PackageParser.hpp"
#include "Class.hpp"
#include "Function.hpp"
#include "Enum.hpp"
#include "ValueType.hpp"
#include "Protocol.hpp"
#include "TypeContext.hpp"

void PackageParser::parse() {
    while (stream_.hasMoreTokens()) {
        auto documentation = parseDocumentationToken();
        
        auto exported = Attribute<E_EARTH_GLOBE_EUROPE_AFRICA>().parse(&stream_);
        auto theToken = stream_.consumeToken(IDENTIFIER);
        switch (theToken.value[0]) {
            case E_PACKAGE: {
                exported.disallow();
                
                auto nameToken = stream_.consumeToken(VARIABLE);
                auto namespaceToken = stream_.consumeToken(IDENTIFIER);
                
                auto name = nameToken.value.utf8CString();
                package_->loadPackage(name, namespaceToken.value[0], theToken);
                
                continue;
            }
            case E_CROCODILE:
                parseProtocol(documentation, theToken, exported.set());
                continue;
            case E_TURKEY:
                parseEnum(documentation, theToken, exported.set());
                continue;
            case E_RADIO:
                exported.disallow();
                package_->setRequiresBinary();
                if (strcmp(package_->name(), "_") == 0) {
                    throw CompilerErrorException(theToken, "You may not set 📻 for the _ package.");
                }
                continue;
            case E_CRYSTAL_BALL: {
                exported.disallow();
                if (package_->validVersion()) {
                    throw CompilerErrorException(theToken, "Package version already declared.");
                }
                
                auto major = stream_.consumeToken(INTEGER).value.utf8CString();
                auto minor = stream_.consumeToken(INTEGER).value.utf8CString();
                
                uint16_t majori = strtol(major, nullptr, 0);
                uint16_t minori = strtol(minor, nullptr, 0);
                
                delete [] major;
                delete [] minor;
                
                package_->setPackageVersion(PackageVersion(majori, minori));
                if (!package_->validVersion()) {
                    throw CompilerErrorException(theToken, "The provided package version is not valid.");
                }
                
                continue;
            }
            case E_WALE: {
                exported.disallow();
                EmojicodeChar className, enamespace;
                bool optional;
                auto &classNameToken = parseTypeName(&className, &enamespace, &optional);
                
                if (optional) {
                    throw CompilerErrorException(classNameToken, "Optional types are not extendable.");
                }
                
                Type type = typeNothingness;
                
                if (!package_->fetchRawType(className, enamespace, optional, theToken, &type)) {
                    throw CompilerErrorException(classNameToken, "Class does not exist.");
                }
                if (type.type() != TypeContent::Class && type.type() != TypeContent::ValueType) {
                    throw CompilerErrorException(classNameToken, "Only classes and value types are extendable.");
                }
                
                // Native extensions are allowed if the class was defined in this package.
                parseTypeDefinitionBody(type, nullptr, type.eclass()->package() == package_);
                
                continue;
            }
            case E_RABBIT:
                parseClass(documentation, theToken, exported.set());
                continue;
            case E_DOVE_OF_PEACE:
                parseValueType(documentation, theToken, exported.set());
                continue;
            case E_SCROLL: {
                exported.disallow();
                auto pathString = stream_.consumeToken(STRING);
                auto relativePath = pathString.position().file;
                auto fileString = pathString.value.utf8CString();
                
                char *str = fileString;
                const char *lastSlash = strrchr(relativePath, '/');
                if (lastSlash != nullptr) {
                    const char *directory = strndup(relativePath, lastSlash - relativePath);
                    asprintf(&str, "%s/%s", directory, fileString);
                    delete [] fileString;
                    delete [] directory;
                }
                
                PackageParser(package_, lex(str)).parse();
                
                delete [] str;
                continue;
            }
            case E_CHEQUERED_FLAG: {
                if (Function::foundStart) {
                    throw CompilerErrorException(theToken, "Duplicate 🏁.");
                }
                Function::foundStart = true;
                
                auto function = new Function(E_CHEQUERED_FLAG, PUBLIC, false, typeNothingness, package_, theToken,
                                              false, documentation, false);
                parseReturnType(function, typeNothingness);
                if (function->returnType.type() != TypeContent::Nothingness &&
                    !function->returnType.compatibleTo(typeInteger, typeNothingness)) {
                    throw CompilerErrorException(theToken, "🏁 must either return ✨ or 🚂.");
                }
                parseBody(function, false);
                
                Function::start = function;
                break;
            }
            default:
                ecCharToCharStack(theToken.value[0], f);
                throw CompilerErrorException(theToken, "Unexpected identifier %s", f);
                break;
        }
    }
}

void PackageParser::reservedEmojis(const Token &token, const char *place) const {
    EmojicodeChar name = token.value[0];
    switch (name) {
        case E_CUSTARD:
        case E_DOUGHNUT:
        case E_SHORTCAKE:
        case E_CHOCOLATE_BAR:
        case E_COOKING:
        case E_COOKIE:
        case E_LOLLIPOP:
        case E_CLOCKWISE_RIGHTWARDS_AND_LEFTWARDS_OPEN_CIRCLE_ARROWS:
        case E_CLOCKWISE_RIGHTWARDS_AND_LEFTWARDS_OPEN_CIRCLE_ARROWS_WITH_CIRCLED_ONE_OVERLAY:
        case E_RED_APPLE:
        case E_BEER_MUG:
        case E_CLINKING_BEER_MUGS:
        case E_LEMON:
        case E_GRAPES:
        case E_STRAWBERRY:
        case E_BLACK_SQUARE_BUTTON:
        case E_LARGE_BLUE_DIAMOND:
        case E_DOG:
        case E_HIGH_VOLTAGE_SIGN:
        case E_CLOUD:
        case E_BANANA:
        case E_HONEY_POT:
        case E_SOFT_ICE_CREAM:
        case E_ICE_CREAM:
        case E_TANGERINE:
        case E_WATERMELON:
        case E_AUBERGINE:
        case E_SPIRAL_SHELL:
        case E_BLACK_RIGHT_POINTING_DOUBLE_TRIANGLE:
        case E_BLACK_RIGHT_POINTING_DOUBLE_TRIANGLE_WITH_VERTICAL_BAR: {
            ecCharToCharStack(name, nameString);
            throw CompilerErrorException(token, "%s is reserved and cannot be used as %s name.", nameString, place);
        }
    }
}

//MARK: Utilities

const Token& PackageParser::parseAndValidateNewTypeName(EmojicodeChar *name, EmojicodeChar *ns) {
    bool optional;
    auto &nameToken = parseTypeName(name, ns, &optional);
    
    if (optional) {
        throw CompilerErrorException(nameToken, "🍬 cannot be declared as type.");
    }
    
    Type type = typeNothingness;
    if (package_->fetchRawType(*name, *ns, optional, nameToken, &type)) {
        auto str = type.toString(typeNothingness, true);
        throw CompilerErrorException(nameToken, "Type %s is already defined.", str.c_str());
    }
    
    return nameToken;
}

void PackageParser::parseGenericArgumentList(TypeDefinitionFunctional *typeDef, TypeContext tc) {
    while (stream_.nextTokenIs(E_SPIRAL_SHELL)) {
        stream_.consumeToken(IDENTIFIER);
        
        auto variable = stream_.consumeToken(VARIABLE);
        auto constraintType = parseTypeDeclarative(tc, TypeDynamism::None, typeNothingness, nullptr, true);
        typeDef->addGenericArgument(variable, constraintType);
    }
}

static AccessLevel readAccessLevel(TokenStream *stream) {
    AccessLevel access = PUBLIC;
    if (stream->nextTokenIs(E_CLOSED_LOCK_WITH_KEY)) {
        access = PROTECTED;
        stream->consumeToken(IDENTIFIER);
    }
    else if (stream->nextTokenIs(E_LOCK)) {
        access = PRIVATE;
        stream->consumeToken(IDENTIFIER);
    }
    else if (stream->nextTokenIs(E_OPEN_LOCK)) {
        stream->consumeToken(IDENTIFIER);
    }
    return access;
}

void PackageParser::parseProtocol(const EmojicodeString &documentation, const Token &theToken, bool exported) {
    EmojicodeChar name, enamespace;
    parseAndValidateNewTypeName(&name, &enamespace);
    
    auto protocol = new Protocol(name, package_, theToken, documentation);
    
    parseGenericArgumentList(protocol, Type(protocol, false));
    protocol->finalizeGenericArguments();
    
    auto token = stream_.consumeToken(IDENTIFIER);
    if (token.value[0] != E_GRAPES) {
        ecCharToCharStack(token.value[0], s);
        throw CompilerErrorException(token, "Expected 🍇 but found %s instead.", s);
    }
    
    auto protocolType = Type(protocol, false);
    package_->registerType(protocolType, name, enamespace, exported);
    
    while (stream_.nextTokenIsEverythingBut(E_WATERMELON)) {
        auto documentation = parseDocumentationToken();
        auto token = stream_.consumeToken(IDENTIFIER);
        auto deprecated = Attribute<E_WARNING_SIGN>().parse(&stream_);
        
        if (token.value[0] != E_PIG) {
            throw CompilerErrorException(token, "Only method declarations are allowed inside a protocol.");
        }
        
        auto methodName = stream_.consumeToken(IDENTIFIER);
        
        auto method = new Method(methodName.value[0], PUBLIC, false, typeNothingness, package_, methodName.position(),
                                 false, documentation, deprecated.set());
        auto a = parseArgumentList(method, protocolType);
        auto b = parseReturnType(method, protocolType);
        if (a || b) {
            protocol->setUsesSelf();
        }
        
        protocol->addMethod(method);
    }
    stream_.consumeToken(IDENTIFIER);
}

void PackageParser::parseEnum(const EmojicodeString &documentation, const Token &theToken, bool exported) {
    EmojicodeChar name, enamespace;
    parseAndValidateNewTypeName(&name, &enamespace);
    
    Enum *eenum = new Enum(name, package_, theToken, documentation);
    
    package_->registerType(Type(eenum, false), name, enamespace, exported);
    
    auto token = stream_.consumeToken(IDENTIFIER);
    if (token.value[0] != E_GRAPES) {
        ecCharToCharStack(token.value[0], s);
        throw CompilerErrorException(token, "Expected 🍇 but found %s instead.", s);
    }
    while (stream_.nextTokenIsEverythingBut(E_WATERMELON)) {
        eenum->addValueFor(stream_.consumeToken().value[0]);
    }
    stream_.consumeToken(IDENTIFIER);
}

void PackageParser::parseClass(const EmojicodeString &documentation, const Token &theToken, bool exported) {
    EmojicodeChar name, enamespace;
    parseAndValidateNewTypeName(&name, &enamespace);
    
    auto eclass = new Class(name, package_, theToken, documentation);
    
    parseGenericArgumentList(eclass, Type(eclass));
    
    if (!stream_.nextTokenIs(E_GRAPES)) {
        EmojicodeChar typeName, typeNamespace;
        bool optional;
        auto &token = parseTypeName(&typeName, &typeNamespace, &optional);
        
        Type type = typeNothingness;
        if (!package_->fetchRawType(typeName, typeNamespace, optional, token, &type)) {
            throw CompilerErrorException(token, "Superclass type does not exist.");
        }
        if (type.type() != TypeContent::Class) {
            throw CompilerErrorException(token, "The superclass must be a class.");
        }
        if (type.optional()) {
            throw CompilerErrorException(token, "An 🍬 is not a valid superclass.");
        }
        
        eclass->superclass = type.eclass();
        
        eclass->setSuperTypeDef(eclass->superclass);
        parseGenericArgumentsForType(&type, Type(eclass), TypeDynamism::GenericTypeVariables, token);
        eclass->setSuperGenericArguments(type.genericArguments);
    }
    else {
        eclass->superclass = nullptr;
        eclass->finalizeGenericArguments();
    }
    
    package_->registerType(Type(eclass), name, enamespace, exported);
    package_->registerClass(eclass);
    
    std::set<EmojicodeChar> requiredInitializers;
    if (eclass->superclass != nullptr) {
        // This set contains methods that must be implemented.
        // If a method is implemented it gets removed from this list by parseClassBody.
        requiredInitializers = std::set<EmojicodeChar>(eclass->superclass->requiredInitializers());
    }
    
    parseTypeDefinitionBody(Type(eclass), &requiredInitializers, true);
    
    if (requiredInitializers.size()) {
        ecCharToCharStack(*requiredInitializers.begin(), name);
        throw CompilerErrorException(eclass->position(), "Required initializer %s was not implemented.", name);
    }
}

void PackageParser::parseValueType(const EmojicodeString &documentation, const Token &theToken, bool exported) {
    EmojicodeChar name, enamespace;
    parseAndValidateNewTypeName(&name, &enamespace);
    
    auto valueType = new ValueType(name, package_, theToken, documentation);
    
    auto valueTypeContent = Type(valueType, false);
    
    parseGenericArgumentList(valueType, valueTypeContent);
    valueType->finalizeGenericArguments();
    
    package_->registerType(valueTypeContent, name, enamespace, exported);
    parseTypeDefinitionBody(valueTypeContent, nullptr, true);
}

void PackageParser::parseTypeDefinitionBody(Type typed, std::set<EmojicodeChar> *requiredInitializers,
                                            bool allowNative) {
    allowNative = allowNative && package_->requiresBinary();
    
    auto token = stream_.consumeToken(IDENTIFIER);
    if (token.value[0] != E_GRAPES) {
        ecCharToCharStack(token.value[0], s);
        throw CompilerErrorException(token, "Expected 🍇 but found %s instead.", s);
    }
    
    while (stream_.nextTokenIsEverythingBut(E_WATERMELON)) {
        auto documentation = parseDocumentationToken();
        
        auto deprecated = Attribute<E_WARNING_SIGN>().parse(&stream_);
        auto final = Attribute<E_LOCK_WITH_INK_PEN>().parse(&stream_);
        AccessLevel accessLevel = readAccessLevel(&stream_);
        auto override = Attribute<E_BLACK_NIB>().parse(&stream_);
        auto staticOnType = Attribute<E_RABBIT>().parse(&stream_);
        auto required = Attribute<E_KEY>().parse(&stream_);
        auto canReturnNothingness = Attribute<E_CANDY>().parse(&stream_);
        
        auto eclass = typed.type() == TypeContent::Class ? typed.eclass() : nullptr;
        
        auto token = stream_.consumeToken(IDENTIFIER);
        switch (token.value[0]) {
            case E_SHORTCAKE: {
                if (!eclass) {
                    throw CompilerErrorException(token, "🍰 not allowed here.");
                }
                
                staticOnType.disallow();
                override.disallow();
                final.disallow();
                required.disallow();
                canReturnNothingness.disallow();
                deprecated.disallow();

                auto &variableName = stream_.consumeToken(VARIABLE);
                auto type = parseTypeDeclarative(typed, TypeDynamism::GenericTypeVariables);
                eclass->addInstanceVariable(Variable(type, 0, 1, false, variableName));
                break;
            }
            case E_CROCODILE: {
                if (!eclass) {
                    throw CompilerErrorException(token, "🐊 not supported yet.");
                }
                
                staticOnType.disallow();
                override.disallow();
                final.disallow();
                required.disallow();
                canReturnNothingness.disallow();
                deprecated.disallow();
                
                Type type = parseTypeDeclarative(typed, TypeDynamism::GenericTypeVariables, typeNothingness, nullptr,
                                                 true);
                
                if (type.optional()) {
                    throw CompilerErrorException(token, "A class cannot conform to an 🍬 protocol.");
                }
                if (type.type() != TypeContent::Protocol) {
                    throw CompilerErrorException(token, "The given type is not a protocol.");
                }
                
                eclass->addProtocol(type);
                break;
            }
            case E_PIG: {
                required.disallow();
                canReturnNothingness.disallow();

                auto methodName = stream_.consumeToken(IDENTIFIER);
                EmojicodeChar name = methodName.value[0];
                
                if (staticOnType.set()) {
                    auto *classMethod = new ClassMethod(name, accessLevel, final.set(), typed, package_,
                                                        token.position(), override.set(), documentation,
                                                        deprecated.set());
                    auto context = TypeContext(typed, classMethod);
                    parseGenericArgumentsInDefinition(classMethod, context);
                    parseArgumentList(classMethod, context);
                    parseReturnType(classMethod, context);
                    parseBody(classMethod, allowNative);
                    
                    typed.typeDefinitionFunctional()->addClassMethod(classMethod);
                }
                else {
                    reservedEmojis(methodName, "method");
                    
                    auto *method = new Method(methodName.value[0], accessLevel, final.set(), typed,
                                              package_, token.position(), override.set(), documentation,
                                              deprecated.set());
                    auto context = TypeContext(typed, method);
                    parseGenericArgumentsInDefinition(method, context);
                    parseArgumentList(method, context);
                    parseReturnType(method, context);
                    parseBody(method, allowNative);
                    
                    typed.typeDefinitionFunctional()->addMethod(method);
                }
                break;
            }
            case E_CAT: {
                staticOnType.disallow();
                
                EmojicodeChar name = stream_.consumeToken(IDENTIFIER).value[0];
                Initializer *initializer = new Initializer(name, accessLevel, final.set(), typed, package_,
                                                           token.position(), override.set(), documentation,
                                                           deprecated.set(), required.set(),
                                                           canReturnNothingness.set());
                auto context = TypeContext(typed, initializer);
                parseGenericArgumentsInDefinition(initializer, context);
                parseArgumentList(initializer, context);
                parseBody(initializer, allowNative);
                
                if (requiredInitializers) {
                    requiredInitializers->erase(name);
                }
                
                typed.typeDefinitionFunctional()->addInitializer(initializer);
                break;
            }
            default: {
                ecCharToCharStack(token.value[0], cs);
                throw CompilerErrorException(token, "Unexpected identifier %s.", cs);
                break;
            }
        }
    }
    stream_.consumeToken(IDENTIFIER);
}

EmojicodeString PackageParser::parseDocumentationToken() {
    EmojicodeString documentation;
    if (stream_.nextTokenIs(DOCUMENTATION_COMMENT)) {
        documentation = stream_.consumeToken(DOCUMENTATION_COMMENT).value;
    }
    return documentation;
}