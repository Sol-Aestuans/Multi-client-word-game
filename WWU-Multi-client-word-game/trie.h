
#define TRIE_ALPHA_SIZE 26

typedef struct node {

    // TODO: Define fields of the node
    struct node* children[TRIE_ALPHA_SIZE];
    int is_word;

} trie_node;

// TODO: Implement the following functions in trie.c
trie_node *trie_create(void);
int trie_insert(trie_node *trie, char *word, unsigned int word_len);
int trie_search(trie_node *trie, char *word, unsigned int word_len);
void trie_delete(trie_node *trie, char *word, unsigned int word_len);