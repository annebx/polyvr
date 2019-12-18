#include "xml.h"
#include "toString.h"

#include <iostream>
#include <libxml/tree.h>
#include <libxml/parser.h>


XMLElement::XMLElement(_xmlNode* node) : node(node) {}
XMLElement::~XMLElement() {}

XMLElementPtr XMLElement::create(_xmlNode* node) { return XMLElementPtr( new XMLElement(node) ); }

void XMLElement::print() {
    if (!node) cout << "invalid xml node!";
    else {
        cout << "xml node: '" << getName() << "', ns: '" << getNameSpace() << "', data: " << getText();
        for (auto a : getAttributes()) cout << ", " << a.first << ":" << a.second;
    }
    cout << endl;
}

string XMLElement::getName() {
    if (!node || ! node->name) return "";
    return string((const char*)node->name);
}

string XMLElement::getNameSpace() {
    if (!node || ! node->ns) return "";
    return string((const char*)node->ns);
}

string XMLElement::getText() {
    if (!hasText()) return "";
    auto txt = xmlNodeGetContent( node->children );
    auto res = string((const char*)txt);
    xmlFree(txt);
    return res;
}

bool XMLElement::hasText() {
    if (!node || !node->children) return false;
    auto txt = xmlNodeGetContent( node->children );
    if (!txt) return false;
    return true;
}

string XMLElement::getAttribute(string name) {
    auto a = xmlGetProp(node, (xmlChar*)name.c_str());
    string v = a ? string((const char*)a) : "";
    if (a) xmlFree(a);
    return v;
}

bool XMLElement::hasAttribute(string name) {
    auto a = xmlHasProp(node, (xmlChar*)name.c_str());
    return a != 0;
}

map<string,string> XMLElement::getAttributes() {
    map<string,string> res;
    if (!node) return res;
    xmlAttr* attribute = node->properties;
    while(attribute) {
        xmlChar* value = xmlNodeListGetString(node->doc, attribute->children, 1);
        string ans((const char*)attribute->name);
        string vas((const char*)value);
        res[ans] = vas;
        xmlFree(value);
        attribute = attribute->next;
    }
    return res;
}

void XMLElement::setAttribute(string name, string value) {
    xmlSetProp(node, (xmlChar*)name.c_str(), (xmlChar*)value.c_str());
}

_xmlNode* getNextNode(_xmlNode* cur) {
    while ( cur && xmlIsBlankNode(cur) ) cur = cur->next;
    return cur;
}

vector<XMLElementPtr> XMLElement::getChildren(string name) {
    vector<XMLElementPtr> res;
    auto cnode = getNextNode( node->xmlChildrenNode );
    while (cnode) {
        if (cnode->type != XML_ELEMENT_NODE) continue;
        if (name != "" && name != string((const char*)cnode->name)) continue;
        res.push_back(XMLElement::create(cnode));
        cnode = getNextNode( cnode->next );
    }
    return res;
}

XMLElementPtr XMLElement::getChild(string name) {
    auto cnode = getNextNode( node->xmlChildrenNode );
    while (cnode) {
        if (cnode->type != XML_ELEMENT_NODE) continue;
        if (name == string((const char*)cnode->name)) return XMLElement::create(cnode);
        cnode = getNextNode( cnode->next );
    }
    return 0;
}

XMLElementPtr XMLElement::getChild(int i) {
    int k = 0;
    auto cnode = getNextNode( node->xmlChildrenNode );
    while (cnode) {
        if (cnode->type != XML_ELEMENT_NODE) continue;
        if (k == i) return XMLElement::create(cnode);
        cnode = getNextNode( cnode->next );
        k++;
    }
    return 0;
}

XMLElementPtr XMLElement::addChild(string name) {
    auto child = xmlNewNode(NULL, (xmlChar*)name.c_str());
    xmlAddChild(node, child);
    return XMLElement::create(child);
}

void XMLElement::setText(string text) {
    auto child = xmlNewText((xmlChar*)text.c_str());
    xmlAddChild(node, child);
}

void XMLElement::importNode(XMLElementPtr e, bool recursive, XML& xml) {
    if (!node) return;
    auto inode = xmlDocCopyNode(e->node, xml.doc, recursive);
    xmlAddChild(node, inode);
}




XML::XML() {}
XML::~XML() {
    if (doc) xmlFreeDoc(doc);
}

XMLPtr XML::create() { return XMLPtr( new XML() ); }

void XML::read(string path, bool validate) {
    if (doc) xmlFreeDoc(doc);
    // parser.set_validate(false); // TODO!
    doc = xmlParseFile(path.c_str());
    xmlNodePtr xmlRoot = xmlDocGetRootElement(doc);
    root = XMLElement::create(xmlRoot);
}

void XML::parse(string data, bool validate) {
    if (doc) xmlFreeDoc(doc);
    // parser.set_validate(false); // TODO!
    doc = xmlParseMemory(data.c_str(), data.size());
    xmlNodePtr xmlRoot = xmlDocGetRootElement(doc);
    root = XMLElement::create(xmlRoot);
}

XMLElementPtr XML::getRoot() { return root; }

XMLElementPtr XML::newRoot(string name, string ns_uri, string ns_prefix) {
    if (doc) xmlFreeDoc(doc);
    doc = xmlNewDoc(NULL);
    auto ns = xmlNewNs(NULL, (xmlChar*)ns_uri.c_str(), (xmlChar*)ns_prefix.c_str());
    auto rnode = xmlNewNode(ns, (xmlChar*)name.c_str());
    root = XMLElement::create( rnode );
    xmlDocSetRootElement(doc, rnode);
    return root;
}

void XML::printTree(XMLElementPtr e, string D) {
    cout << D << e->getName() << endl;
    for (auto c : e->getChildren()) printTree(c, D + " ");
}

string getXMLError() {
    auto error = xmlGetLastError();
    if (!error || error->code == XML_ERR_OK) return ""; // No error

    string str;

    if (error->file && *error->file != '\0') { str += "File "; str += error->file; }

    if (error->line > 0) {
        str += (str.empty() ? "Line " : ", line ") + toString(error->line);
        if (error->int2 > 0) str += ", column " + toString(error->int2);
    }

    const bool two_lines = !str.empty();
    if (two_lines) str += ' ';

    switch (error->level) {
        case XML_ERR_WARNING:
            str += "(warning):";
            break;
        case XML_ERR_ERROR:
            str += "(error):";
            break;
        case XML_ERR_FATAL:
            str += "(fatal):";
            break;
        default:
            str += "():";
            break;
    }

    str += two_lines ? '\n' : ' ';
    if (error->message && *error->message != '\0') str += error->message;
    else str += "Error code " + toString(error->code);
    if (*str.rbegin() != '\n') str += '\n';
    return str;
}

void XML::write(string path) {
    auto r = xmlDocGetRootElement(doc);
    if (!r) { cout << "XML::write failed, no root: " << path << endl; return; }
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlResetLastError();
    const int result = xmlSaveFormatFileEnc(path.c_str(), doc, "UTF-8", 1);
    if (result == -1) cout << "XML::write error: " << getXMLError() << endl;
}

string XML::toString() {
    xmlKeepBlanksDefault(0);
    xmlBuffer* buffer = xmlBufferCreate();
    xmlOutputBuffer* outputBuffer = xmlOutputBufferCreateBuffer( buffer, NULL );
    xmlSaveFormatFileTo(outputBuffer, doc, "UTF-8", 1);
    string str( (char*) buffer->content, buffer->use );
    xmlBufferFree( buffer );
    return str;
}


// SAX parser

XMLStreamHandler::XMLStreamHandler() {}
XMLStreamHandler::~XMLStreamHandler() {}

string toString(const xmlChar* s) {
    if (!s) return "";
    return string((const char*)s);
}

void startDocument(void* user_data) {
    auto handler = (XMLStreamHandler*)user_data;
    handler->startDocument();
}

void endDocument(void* user_data) {
    auto handler = (XMLStreamHandler*)user_data;
    handler->endDocument();
}

void startElement(void* user_data, const xmlChar* name, const xmlChar** attrs) {
    auto handler = (XMLStreamHandler*)user_data;
    string n = toString(name);
    map<string, string> attribs;

    while (attrs != 0 && attrs[0] != 0) {
        string key = toString(attrs[0]);
        string val = toString(attrs[1]);
        attribs[key] = val;
        attrs = &attrs[2];
    }

    handler->startElement(n, attribs);
}

void endElement(void* user_data, const xmlChar* name) {
    auto handler = (XMLStreamHandler*)user_data;
    string n = toString(name);
    handler->endElement(n);
}

void internalSubset(void * ctx, const xmlChar * name, const xmlChar * ExternalID, const xmlChar * SystemID) {}
int isStandalone(void * ctx) { return 1; }
int hasInternalSubset(void * ctx) { return 0; }
int hasExternalSubset(void * ctx) { return 0; }
xmlParserInputPtr resolveEntity(void * ctx, const xmlChar * publicId, const xmlChar * systemId) { return 0; }
xmlEntityPtr getEntity(void * ctx, const xmlChar * name) { return 0; }
void entityDecl(void * ctx, const xmlChar * name, int type, const xmlChar * publicId, const xmlChar * systemId, xmlChar * content) {}
void notationDecl(void * ctx, const xmlChar * name, const xmlChar * publicId, const xmlChar * systemId) {}
void attributeDecl(void * ctx, const xmlChar * elem, const xmlChar * fullname, int type, int def, const xmlChar * defaultValue, xmlEnumerationPtr tree) {}
void elementDecl(void * ctx, const xmlChar * name, int type, xmlElementContentPtr content) {}
void unparsedEntityDecl(void * ctx, const xmlChar * name, const xmlChar * publicId, const xmlChar * systemId, const xmlChar * notationName) {}
void setDocumentLocator(void * ctx, xmlSAXLocatorPtr loc) {}
void reference(void * ctx, const xmlChar * name) {}
void characters(void * ctx, const xmlChar * ch, int len) {}
void ignorableWhitespace(void * ctx, const xmlChar * ch, int len) {}
void processingInstruction(void * ctx, const xmlChar * target, const xmlChar * data) {}
void comment(void * ctx, const xmlChar * value) {}
void warning(void * ctx, const char * msg, ...) {}
void error(void * ctx, const char * msg, ...) {}
void fatalError(void * ctx, const char * msg, ...) {}

static xmlSAXHandler xmlHandler {
    internalSubset,
    isStandalone,
    hasInternalSubset,
    hasExternalSubset,
    resolveEntity,
    getEntity,
    entityDecl,
    notationDecl,
    attributeDecl,
    elementDecl,
    unparsedEntityDecl,
    setDocumentLocator,
    startDocument,
    endDocument,
    startElement,
    endElement,
    reference,
    characters,
    ignorableWhitespace,
    processingInstruction,
    comment,
    warning,
    error,
    fatalError,
};

void XML::stream(string path, XMLStreamHandler* handler) {
    try {
        xmlSAXUserParseFile( &xmlHandler, handler, path.c_str() );
    } catch(exception& e) {
        cout << "meh, streaming '" << path << "' failed with: " << e.what() << endl;
    }
}






