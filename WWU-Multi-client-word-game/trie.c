/* trie.c - code for trie data structure.
 * 1) DO NOT rename this file
 * 2) DO NOT add main() to this file. Create a separate file to write the driver main function.
 * */

#include "trie.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define ASCII_INDEX 97

/**
 * Create an empty trie
 *
 * @return Pointer to the root node.
 *
 */
trie_node *trie_create(void) {
    trie_node* root = (trie_node*) malloc(sizeof(trie_node));

    root->is_word = 0; // set is_word to false

    for(int i = 0; i < TRIE_ALPHA_SIZE; i++){ // set all children to null
      root->children[i] = NULL;
    }

    return root;
}

/**
 * Insert a word in the given trie.
 *
 * @param root node of a trie
 * @param word to insert
 * @return 1 on success, 0 on failure. If the word already exists, return 1.
 */
int trie_insert(trie_node *root, char *word, unsigned int word_len) {

  if(root == NULL){ // root is null
    return 0;
  } else {

    trie_node* cur_node = root;

    for (int i = 0; i < word_len; i++) { // travel through the trie as deep as the word_len
      int trie_index = word[i] - ASCII_INDEX;
        
      if(trie_index <= 25 && trie_index >= 0){ // character is in bounds (lowercase letter)

        if (cur_node->children[trie_index] == NULL) { // if there's no node, create one
          cur_node->children[trie_index] = trie_create();

          if(cur_node->children[trie_index] == NULL){ // error creating node
            return 0;
          }
        }
        cur_node = cur_node->children[trie_index];

      } else { // character is out of bounds
        return 0;
      }
    }

    if(cur_node->is_word == 0){ // new word added
      cur_node->is_word = 1;
    }
    return 1; //success
  }
}


/**
 * Search a word in the given trie. Return 1 if word exists, otherwise return 0.
 *
 * @param root node of a trie
 * @param word Word to search in the trie
 * @param word_len Length of the word
 * @return 1 if the word exists in the trie, otherwise returns 0
 */
int trie_search(trie_node *root, char *word, unsigned int word_len) {

  if(root == NULL){ // root is null
    return 0;
  } else {

    trie_node* cur_node = root;

    for (int i = 0; i < word_len; i++) { // travel through the trie as deep as the word_len
      int trie_index = word[i] - ASCII_INDEX;
        
      if (trie_index <= 25 && trie_index >= 0) { // character is in bounds (lowercase letter)

        if (cur_node->children[trie_index] == NULL) { // word doesn't exist
          return 0;
        }
        cur_node = cur_node->children[trie_index];

      } else { // character is out of bounds, word doesn't exist
        return 0;
      }
    }

    if (cur_node->is_word == 1) { // word exists
      return 1;
    } else { // word doesn't exist
      return 0;
    }
  }
}


/**
 * Delete a word in the given trie.
 *
 * @param root node of a trie
 * @param word Word to search in the trie
 * @param word_len Length of the word
 */
void trie_delete(trie_node *root, char *word, unsigned int word_len) {

  if(root == NULL){ // root is null
    return;
  } else {

    trie_node* cur_node = root;

    for (int i = 0; i < word_len; i++) { // travel through the trie as deep as the word_len
      int trie_index = word[i] - ASCII_INDEX;
        
      if (trie_index <= 25 && trie_index >= 0) { // character is in bounds (lowercase letter)

        if (cur_node->children[trie_index] == NULL) { // word doesn't exist
          return;
        }
        cur_node = cur_node->children[trie_index];

      } else { // character is out of bounds, word doesn't exist
        return;
      }
    }

    if (cur_node->is_word == 1) { // word exists
      cur_node->is_word = 0; // remove word from trie
      return;
    } else { // word doesn't exist
      return;
    }
  }
}
