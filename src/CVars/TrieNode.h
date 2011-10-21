/*

    Cross platform "CVars" functionality.

    This Code is covered under the LGPL.  See COPYING file for the license.
   
    $Id: TrieNode.h 56 2008-09-28 20:08:38Z effer $

 */


#ifndef _TRIE_NODE_H__
#define _TRIE_NODE_H__

#include <string>
#include <list>
#include <vector>

enum TrieNodeType
{
    TRIE_LEAF=0,
    TRIE_NODE,
    TRIE_ROOT
};

class TrieNode
{
 public:
    TrieNode();
    TrieNode( TrieNodeType t );
    TrieNode( std::string s );
    TrieNode( char c );
    ~TrieNode(); //destroy all nodes
 
    TrieNode*             TraverseInsert( char addchar );
    TrieNode*             TraverseFind( char addchar );
    void                  PrintToVector( std::vector<std::string> &vec );
    void                  PrintNodeToVector( std::vector<TrieNode*> &vec );

    void*                 m_pNodeData;
    std::list<TrieNode*>  m_children;
    TrieNodeType          m_nNodeType;

    // private:
    std::string           m_sLeafText;
    char                  m_cNodeChar;
};

#endif
