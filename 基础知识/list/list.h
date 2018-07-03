#ifndef __MY_LIST__
#define __MY_LIST__

#define __DEBUG__

#ifdef __DEBUG__
//#define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)
#define MY_ERR(format,...) printf("FUNC:%s, LINE:%d :"format"\n", __func__, __LINE__, ##__VA_ARGS__)
#define PRINT(format,...) printf("%s: "format"\n", __func__, ##__VA_ARGS__)
#else

#define DEBUG(format, ...)

#endif


typedef struct node {
    int a;
    struct node *next;
} node_t;

node_t * create_list(int n);
node_t * destroy_list(int n);

unsigned int list_size();

void print_list(node_t *head);

//add nodes
node_t *add_to_list();
node_t *add_to_head();
node_t *add_to_mid();
node_t *add_to_tail();

//delete nodes
node_t *del_from_list();
node_t *del_from_head();
node_t *del_from_mid();
node_t *del_from_tail();

//sort nodes
node_t *list_sort();
node_t *sort_bigger();
node_t *sort_smaller();



#endif
