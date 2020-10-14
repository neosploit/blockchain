#include <string.h>
#include <jansson.h>
#include "auxilary.h"
#include "json.h"

// Node

json_t *json_construct_node(const node_t *node) {
    json_t *json_node = json_object();

    if (json_object_set(json_node, "ip_address", json_string(node->ip_address)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_node/json_object_set (ip_address)\n");
        return NULL;
    }

    if (json_object_set(json_node, "port", json_integer(node->port)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_node/json_object_set (port)\n");
        return NULL;
    }

    return json_node;
}

node_t json_destruct_node(const json_t *json_node) {
    node_t node;

    node.ip_address = json_string_value(json_object_get(json_node, "ip_address"));
    node.port = json_integer_value(json_object_get(json_node, "port"));

    return node;
}

bool json_compare_nodes(const json_t *json_node1, const json_t *json_node2) {
    const node_t node1 = json_destruct_node(json_node1);
    const node_t node2 = json_destruct_node(json_node2);

    if (strcmp(node1.ip_address, node2.ip_address)) return false;
    if (node1.port != node2.port) return false;

    return true;
}

bool json_node_is_valid(const json_t *json_node) {
    if (!json_is_object(json_node)) return false;
    if (json_object_size(json_node) != 2) return false;

    // 1: IP address
    if (!json_object_get(json_node, "ip_address")) return false;
    if (!json_is_string(json_object_get(json_node, "ip_address"))) return false;
    if (!ip_address_is_valid(json_string_value(json_object_get(json_node, "ip_address")))) return false;

    // 2: Port
    if (!json_object_get(json_node, "port")) return false;
    if (!json_is_integer(json_object_get(json_node, "port"))) return false;
    if (!port_is_valid(json_integer_value(json_object_get(json_node, "port")))) return false;

    return true;
}

// Extra

bool json_request_is_valid(const json_t *request_body, json_object_type_t object_type) {
    if (!json_is_object(request_body) && !json_is_array(request_body)) return false;

    if ((json_is_object(request_body) && !json_object_is_valid(request_body, object_type)) ||
        (json_is_array(request_body) && !json_array_is_valid(request_body, object_type))) return false;
    
    return true;
}

bool json_array_is_valid(const json_t *array, json_object_type_t object_type) {
    json_t *value;
    int index;

    if (!json_is_array(array)) return false;

    json_array_foreach(array, index, value) {
        if (!json_object_is_valid(value, object_type)) return false;
    }

    return true;
}

bool json_object_is_valid(const json_t *object, json_object_type_t object_type) {
    if (!json_is_object(object)) return false;

    switch (object_type) {
        case NODE: return json_node_is_valid(object);
        default: return false;
    }
}