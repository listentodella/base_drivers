#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct student {
    struct list_head node;
    unsigned char age;
    char name[32];
};


int main()
{
    LIST_HEAD(class);
    struct student *stu = NULL;
    struct student *tmp = NULL;


    printf(" ============ ADD =============== \n");

    for (int i = 0; i < 5; i++) {
        stu = (struct student *)malloc(sizeof(struct student) * 1);
        if (!stu) {
            printf("failed to malloc for stu[%d]\n", i);
            break;
        }
        stu->age = i;
        snprintf(stu->name, sizeof(stu->name), "hello-world-%d", i);
        list_add_tail(&stu->node, &class);// add to tail
        //list_add(&stu->node, &class);// add to head
    }
    //LIST_HEAD_INIT(stu0.node);


    struct student *pos = NULL;
    list_for_each_entry(pos, &class, node) {
        printf("stu[%s]: age = %d\n", pos->name, pos->age);
        if (list_is_last(&pos->node, &class))
            printf("end of list ...\n");
    }

    printf(" ============ REPLACE =============== \n");

    struct student *new = (struct student *)malloc(sizeof(struct student) * 1);
    if (!new) {
        printf("failed to malloc for new stu\n");
    } else {
        strcpy(new->name, "HELLO-WORLD-x");
        new->age  = 23;
    }

    //list_for_each_entry(pos, &class, node) {
    list_for_each_entry_safe(pos, tmp, &class, node) {
        if (!strcmp(pos->name, "hello-world-2")) {
            //list_replace(&pos->node, &new->node); // match list_for_each_entry work. if match _safe, it will stop...
            list_replace_init(&pos->node, &new->node);

            list_del_init(&pos->node); // safe
            free(pos);
        }
    }
    list_for_each_entry(pos, &class, node) {
        printf("stu[%s]: age = %d\n", pos->name, pos->age);
        if (list_is_last(&pos->node, &class))
            printf("end of list ...\n");
    }


    printf(" ============ DELETE =============== \n");

    //pos = NULL;

    // list_for_each_entry(pos, &class, node) {
    //     printf("remove stu[%s]: age = %d\n", pos->name, pos->age);
    //     list_del(&pos->node);
    //     free(pos);
    // }

    //list_for_each_entry(pos, &class, node) { // if deleted, not safe
    list_for_each_entry_safe(pos, tmp, &class, node) {
        printf("remove stu[%s]: age = %d\n", pos->name, pos->age);
        //list_del(&pos->node); // not safe, but still work
        list_del_init(&pos->node); // safe
        free(pos);
    }

    pos = NULL;
    list_for_each_entry(pos, &class, node) {
        printf("exist stu[%s]: age = %d\n", pos->name, pos->age);
    }

    return 0;
}
