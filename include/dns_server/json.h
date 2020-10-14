#ifndef SERVER_OBJECTS_H
#define SERVER_OBJECTS_H

#include <stdbool.h>
#include <jansson.h>
#include <netinet/in.h>

typedef enum json_object_type_t {
    NODE
} json_object_type_t;

// Node

typedef struct node_t {
    const char *ip_address;
    in_port_t port;
} node_t;

json_t *json_construct_node(const node_t *node);
node_t json_destruct_node(const json_t *json_node);
bool json_compare_nodes(const json_t *json_node1, const json_t *json_node2);
bool json_node_is_valid(const json_t *json_node);

// Extra

bool json_request_is_valid(const json_t *body, json_object_type_t object_type);
bool json_array_is_valid(const json_t *array, json_object_type_t object_type);
bool json_object_is_valid(const json_t *object, json_object_type_t object_type);

#endif // SERVER_OBJECTS_H