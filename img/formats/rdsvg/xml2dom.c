/*
 * This file is an optional part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 *
 * It allows you to use libXml2 instead of dom as the xml parsing library.
 *
 * Copyright 2016 by David Phillip Oster
 */
#include "xml2dom.h"

#include <assert.h>
#include <ctype.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>
#include <string.h>

static int sDidInitXML2Lib = 0;

enum
{ MAGIC_DOCUMENT_NODE =  10000
};

static void lower(char *s)
{ for (; *s; ++s)
  { if (isupper(*s))
    { *s = tolower(*s);
} } }

static char *my_strdup(const uint8_t *data, size_t len)
{ char *result = (char *)malloc(len+1);
  memcpy(result, data, len);
  result[len] = '\0';
  return result;
}


dom_xml_parser *dom_xml_parser_create( void *dontCare1
                                     , void *dontCare2
                                     , MesgFuncPtr mesgFunc
                                     , void *dontCare3
                                     , dom_document **outDocument)
{ if (!sDidInitXML2Lib)
  { xmlInitParser();
    sDidInitXML2Lib = 1;
  }

  dom_document *docResult = (dom_document *)calloc(sizeof(dom_document), 1);
  *outDocument = docResult;
  dom_xml_parser *result = (dom_xml_parser *)calloc(sizeof(dom_xml_parser), 1);
  result->doc = docResult;
  return result;
}

dom_xml_error dom_xml_parser_parse_chunk( dom_xml_parser *parser
                                        , const uint8_t *data
                                        , size_t len )
{ assert(parser);

  xmlDoc * doc= xmlReadMemory( (const char *)data
                             , (int)len
                             , NULL, NULL
                             , XML_PARSE_NOCDATA | XML_PARSE_NOBLANKS );

  if ( doc )
  {
    parser->doc->node = (xmlNode *)doc;
    parser->doc->ref = MAGIC_DOCUMENT_NODE;
    return( DOM_XML_OK );
  }

  return( DOM_XML_MALFORMED );
}

dom_xml_error dom_xml_parser_completed(dom_xml_parser *parser)
{
  return DOM_XML_OK;
}

// we intentionally don't free the document here.

dom_xml_error dom_xml_parser_destroy(dom_xml_parser *parser)
{
  free(parser);
  return DOM_XML_OK;
}

dom_exception dom_document_get_document_element( dom_document *document
                                               , dom_element **outNode)
{ dom_element *element = (dom_element *)calloc(sizeof(dom_element), 1);
  element->node = xmlDocGetRootElement((xmlDoc *)(document->node));
  element->ref = 1;
  *outNode = element;

  return( DOM_NO_ERR );
}

static xmlElement * getElementById(xmlElement *element, const xmlChar *idValue)
{
  xmlAttrPtr attrPtr = xmlHasProp((xmlNode *)element, (const xmlChar *)"id");

  if ( attrPtr
    && attrPtr->children
    && attrPtr->children->content
    && !strcasecmp((const char *)attrPtr->children->content
                  ,(const char *)idValue))
  {
    return element;
  }

  xmlElement *result;
  if (element->next && NULL != (result = getElementById((xmlElement *)element->next, idValue)))
  {
    return( result );
  }

  if (element->children && NULL != (result = getElementById((xmlElement *)element->children, idValue)))
  {
    return( result );
  }

  return( NULL );
}

dom_exception dom_document_get_element_by_id( dom_node   *node
                                            , dom_string *string
                                            , dom_element **outNode)
{ xmlAttrPtr attrPtr = xmlHasProp(node->node, (const xmlChar *)"id");

  if (attrPtr && !strcasecmp((const char *)attrPtr->children->content, string->s))
  { node->ref++;
    *outNode = node;
    return DOM_NO_ERR;
  }

  xmlElement *resultXML = getElementById((xmlElement *)node->node, (const xmlChar *)string->s);
  if (resultXML)
  { dom_element *result = (dom_element *)calloc(sizeof(dom_element), 1);
    result->node = (xmlNode *)resultXML;
    result->ref = 1;
    *outNode = result;
    return DOM_NO_ERR;
  }

  *outNode = NULL;
  return DOM_NO_ERR;
}

dom_exception dom_element_get_attribute( dom_node *node
                                       , dom_string *string
                                       , dom_string **outAttribute )
{
  xmlAttrPtr attrPtr = xmlHasProp(node->node, (const xmlChar *)string->s);
  if ( ! attrPtr)
  { if (strchr(string->s, ':'))
    {
      fprintf(stderr, "TODO:dom_element_get_attribute - namespace %s\n", string->s);
  } }
  else
  {
    const char *s = (const char *)attrPtr->children->content;
    return dom_string_create_interned((const uint8_t *)s, strlen(s), outAttribute);
  }
  *outAttribute = NULL;
  return DOM_NO_ERR;
}

dom_exception dom_element_get_elements_by_tag_name( dom_element *element
                                                  , dom_string *string
                                                  , dom_nodelist **outNodeList)
{
  dom_nodelist *result = NULL;
  dom_element **nodeList = NULL;
  int nodeCount = 0;

  for ( xmlElement *candidate = (xmlElement *)element->node->children
      ; candidate
      ; candidate = (xmlElement *)candidate->next)
  {
    if (!strcmp((const char *)candidate->name, string->s))
    {
      if (NULL == nodeList)
      {
        nodeList = (dom_element **)malloc(nodeCount * sizeof(dom_element *));
      }
      else
      {
        dom_element **t = realloc(nodeList, (1+nodeCount) * sizeof(dom_element *));
        if (t)
        {
          nodeList = t;
        }
        else
        {
          for (int i = 0; i < nodeCount; ++i)
          {
            dom_node_unref(nodeList[i]);
          }
          free(nodeList);
          *outNodeList = result;
          return DOM_MEM_ERR;
      } }

      dom_element *elem = (dom_element *)calloc(sizeof(dom_element), 1);
      elem->node = (xmlNode *)candidate;
      elem->ref = 1;
      nodeList[nodeCount++] = elem;
  } }

  if (nodeCount)
  { result = calloc(sizeof(dom_nodelist), 1);
    result->nodes = nodeList;
    result->count = nodeCount;
    result->ref = 1;
  }

  *outNodeList = result;
  return( DOM_NO_ERR );
}

dom_exception dom_node_get_node_name(dom_node *node, dom_string **outString) {
  xmlNode *n = node->node;
  dom_exception errCode = DOM_NO_ERR;

  if (n->ns && n->ns->prefix)
  { char *qname = NULL;
    if (-1 != asprintf(&qname, "%s:%s", n->ns->prefix, n->name))
    {
      errCode = dom_string_create_interned((const uint8_t *)qname, strlen(qname), outString);
      free(qname);
    }
    else
    {
      errCode = DOM_MEM_ERR;
  } }
  else
  {
    errCode = dom_string_create_interned(n->name, strlen((const char *)n->name), outString);
  }
  return errCode;
}

void dom_node_unref( dom_node *node )
{ assert(node);

  if (MAGIC_DOCUMENT_NODE == node->ref)
  { xmlFreeDoc((xmlDoc *)node->node);
    free(node);
  }
  else
  {
    node->ref--;
    if (!node->ref)
    {
      free(node);
} } }

dom_exception dom_nodelist_get_length( dom_nodelist *nodeList
                                     , uint32_t *outLen)
{ *outLen = nodeList->count;
  return DOM_NO_ERR;
}

dom_exception dom_nodelist_item( dom_nodelist *nodeList
                               , uint32_t index
                               , dom_node **outItemp )
{ dom_node *node = nodeList->nodes[index];
  node->ref++;
  *outItemp = node;
  return DOM_NO_ERR;
}

void dom_nodelist_unref(dom_nodelist *nodeList)
{ assert(nodeList);
  nodeList->ref--;

  if (! nodeList->ref )
  { int count = nodeList->count;

    for (int i = 0; i < count; ++i)
    { dom_node_unref(nodeList->nodes[i]);
    }

    if (0 < count)
    { free(nodeList->nodes);
    }
    free(nodeList);
} }

dom_exception dom_node_get_first_child( dom_element *element
                                      , dom_element **outChild )
{ dom_element *newElement= NULL;

  xmlElement *child = (xmlElement *)element->node->children;
  if (child)
  {
    newElement = (dom_element *)calloc(sizeof(dom_element), 1);
    newElement->node = (xmlNode *)child;
    newElement->ref = 1;
  }

  *outChild = newElement;
  return DOM_NO_ERR;
}

dom_exception dom_node_get_node_type( dom_node *node
                                    , dom_node_type *outType)
{ switch (node->node->type)
  { case XML_ELEMENT_NODE:       *outType = DOM_ELEMENT_NODE;   break;
    case XML_ATTRIBUTE_NODE:     *outType = DOM_ATTRIBUTE_NODE; break;
    case XML_CDATA_SECTION_NODE:
    case XML_TEXT_NODE:          *outType = DOM_TEXT_NODE;      break;
    case XML_COMMENT_NODE:       *outType = DOM_COMMENT_NODE;   break;
    case XML_DOCB_DOCUMENT_NODE:
    case XML_DOCUMENT_NODE:
    case XML_DOCUMENT_TYPE_NODE:
    case XML_DOCUMENT_FRAG_NODE:
    case XML_HTML_DOCUMENT_NODE: *outType = DOM_DOCUMENT_NODE; break;
    default:                     *outType = DOM_OTHER_NODE;    break;
  }
  return DOM_NO_ERR;
}

int dom_string_caseless_isequal(dom_string *as, dom_string *bs)
{ return ! strcasecmp(as->s, bs->s);
}

dom_exception dom_node_get_next_sibling( dom_element *element
                                       , dom_element **outChild)
{
  dom_element *nextElement = NULL;
  xmlElement *child = (xmlElement *)element->node->next;

  if (child)
  {
    nextElement = (dom_element *)calloc(sizeof(dom_element), 1);
    nextElement->node = (xmlNode *)child;
    nextElement->ref = 1;
  }

  *outChild = nextElement;
  return DOM_NO_ERR;
}

dom_exception dom_text_get_whole_text( dom_element *element
                                     , dom_string **outString)
{ return DOM_NO_ERR;
}

lwc_error lwc_intern_string(const char *data, size_t len, lwc_string **outString)
{ return (lwc_error)dom_string_create_interned((const uint8_t *)data, len, outString);
}

int dom_string_caseless_lwc_isequal(dom_string *str, lwc_string *lwcString)
{ return dom_string_caseless_isequal(str, lwcString);
}

void lwc_string_unref(lwc_string *lwcString)
{ dom_string_unref(lwcString);
}

uint32_t dom_string_byte_length(dom_string *str)
{ return (uint32_t)strlen(str->s);
}

dom_exception dom_string_create_interned( const uint8_t *data, size_t len
                                        , dom_string **outString )
{ dom_string *newStr = (dom_string *)calloc(sizeof(dom_string), 1);
  newStr->s = my_strdup(data, len);
  newStr->ref = 1;
  *outString = newStr;

  return DOM_NO_ERR;
}

char *dom_string_data(dom_string *str)
{
  return str->s;
}

int dom_string_isequal( dom_string *a, dom_string *b )
{ return !strcmp(a->s, b->s);
}

dom_string * dom_string_ref( dom_string *str )
{ str->ref++;

  return( str );
}

void dom_string_unref(dom_string *str)
{ str->ref--;

  if ( !str->ref )
  { free( str->s );
    free( str    );
} }

char *strduplower2(const uint8_t *data, size_t len)
{ char *lowerStr = my_strdup(data, len);
  lower(lowerStr);
  return lowerStr;
}

char *strduplower(const char *s)
{ return strduplower2((const uint8_t *)s, strlen(s));
}



