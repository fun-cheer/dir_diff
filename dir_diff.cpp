#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <bits/stdc++.h>

typedef struct TREE_NODE {
    char name[256];
    int type;
    int child_num;
    void *child_node;
    bool found;
} TREE_NODE;

static const char *dir_type_name(unsigned char dir_type) {
    switch (dir_type)
    {
    case DT_BLK:
        return "block device";

    case DT_CHR:
        return "character device";

    case DT_DIR:
        return "directory";

    case DT_FIFO:
        return "named pipe";

    case DT_LNK:
        return "symbolic link";

    case DT_REG:
        return "regular file";

    case DT_SOCK:
        return "UNIX domain socket";

    case DT_UNKNOWN:
        return "unknown";

    default:
        return "no such type";
    }
}

static void traverse_dir(const char *name, TREE_NODE *node, int depth = 0) {
    assert(name);
    assert(node);

    // first loop: get child number for current node
    {
        DIR *dp = opendir(name);
        if (dp == nullptr) {
            printf("opendir 1 error: %d(%s)\n", errno, strerror(errno));
            return;
        }

        struct dirent *dent = readdir(dp);
        while (dent) {
            if (dent->d_name[0] != '.') {
                node->child_num++;
            }

            dent = readdir(dp);
        }

        closedir(dp);
        node->child_node = (void *)(new TREE_NODE[node->child_num]);
    }

    // second loop: traverse child node with DT_DIR type
    {
        int id = 0;
        TREE_NODE *pnode = (TREE_NODE *)(node->child_node);
        DIR *dp = opendir(name);
        if (dp == nullptr) {
            printf("opendir 2 error: %d(%s)\n", errno, strerror(errno));
            return;
        }

        struct dirent *dent = readdir(dp);
        while (dent) {
            if (dent->d_name[0] != '.') {
                TREE_NODE *cnode = &(pnode[id]);
                sprintf(cnode->name, "%s", dent->d_name);
                cnode->child_num = 0;
                cnode->child_node = nullptr;
                cnode->found = false;
                cnode->type = dent->d_type;

                if (dent->d_type == DT_DIR) {
                    char path[1024] = {0};
                    sprintf(path, "%s/%s", name, dent->d_name);
                    traverse_dir(path, cnode, depth + 1);
                }

                id++;
            }

            dent = readdir(dp);
        }

        closedir(dp);
    }
}

static void diff_nodes(TREE_NODE *node1, TREE_NODE *node2) {
    //
}

static void print_nodes(TREE_NODE *node, bool found = false, int depth = 0) {
    assert(node);

    for (int i = 0; i < depth; i++) {
        printf("|-- ");
    }

    printf("%s, %s\n", node->name, dir_type_name(node->type));

    if (node->child_node) {
        TREE_NODE *pnode = (TREE_NODE *)(node->child_node);
        for (int i = 0; i < node->child_num; i++) {
            TREE_NODE *cnode = &(pnode[i]);
            print_nodes(cnode, found, depth + 1);
        }
    }
}

static void release_node(TREE_NODE *node) {
    if (node && node->child_num > 0) {
        TREE_NODE *pnode = (TREE_NODE *)(node->child_node);
        for (int i = 0; i < node->child_num; i++) {
            TREE_NODE *cnode = &(pnode[i]);
            release_node(cnode);
        }

        delete [] pnode;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage:\n\t%s dir_path1 dir_path2\n", argv[0]);
        return -1;
    }

    // check path 1
    {
        DIR *dp = opendir(argv[1]);
        if (dp == nullptr) {
            printf("opendir %s error: %d(%s) for root node\n",
                argv[1], errno, strerror(errno));
            return -1;
        }
        closedir(dp);
    }

    // check path 2
    {
        DIR *dp = opendir(argv[2]);
        if (dp == nullptr) {
            printf("opendir %s error: %d(%s) for root node\n",
                argv[2], errno, strerror(errno));
            return -1;
        }
        closedir(dp);
    }

    TREE_NODE *root1 = new TREE_NODE;
    TREE_NODE *root2 = new TREE_NODE;

    sprintf(root1->name, "%s", argv[1]);
    root1->child_num = 0;
    root1->child_node = nullptr;
    root1->found = false;
    root1->type = DT_DIR;

    sprintf(root2->name, "%s", argv[2]);
    root2->child_num = 0;
    root2->child_node = nullptr;
    root2->found = false;
    root2->type = DT_DIR;

    traverse_dir(root1->name, root1);
    traverse_dir(root2->name, root2);

    diff_nodes(root1, root2);

    printf("+++++%s+++++\n", root1->name);
    print_nodes(root1);
    printf("+++++%s+++++\n", root2->name);
    print_nodes(root2);

    release_node(root1);
    release_node(root2);

    delete root1;
    delete root2;

    return 0;
}
