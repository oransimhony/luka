/** @file lib.c */
#include "lib.h"

#include <stdlib.h>

#include "ast.h"

void LIB_free_tokens_vector(t_vector *tokens)
{
    t_token *token = NULL;
    t_iterator iterator = vector_begin(tokens);
    t_iterator last = vector_end(tokens);
    for (; !iterator_equals(&iterator, &last); iterator_increment(&iterator))
    {
        token = *(t_token_ptr *) iterator_get(&iterator);
        if ((token->type == T_NUMBER) || (token->type == T_STRING)
            || ((token->type <= T_IDENTIFIER) && (token->type > T_UNKNOWN)))
        {
            if (NULL != token->content)
            {
                (void) free(token->content);
                token->content = NULL;
            }
        }
        (void) free(token);
        token = NULL;
    }
    (void) vector_clear(tokens);
    (void) vector_destroy(tokens);
    (void) free(tokens);
    tokens = NULL;
}

void lib_free_nodes_vector(t_vector *nodes, t_logger *logger)
{
    t_ast_node *node = NULL;
    t_iterator iterator = vector_begin(nodes);
    t_iterator last = vector_end(nodes);

    for (; !iterator_equals(&iterator, &last); iterator_increment(&iterator))
    {
        node = *(t_ast_node_ptr *) iterator_get(&iterator);
        (void) AST_free_node(node, logger);
    }

    (void) vector_clear(nodes);
    (void) vector_destroy(nodes);
    (void) free(nodes);
    nodes = NULL;
}

t_return_code lib_intialize_nodes(t_vector **nodes, t_logger *logger)
{
    t_return_code status_code = LUKA_UNINITIALIZED;
    *nodes = calloc(1, sizeof(t_vector));
    if (NULL == *nodes)
    {
        (void) LOGGER_log(logger, L_ERROR,
                          "Couldn't allocate memory for nodes");
        status_code = LUKA_CANT_ALLOC_MEMORY;
        goto l_cleanup;
    }

    if (vector_setup(*nodes, 5, sizeof(t_ast_node_ptr)))
    {
        status_code = LUKA_VECTOR_FAILURE;
        goto l_cleanup;
    }

    return LUKA_SUCCESS;

l_cleanup:
    if (NULL != *nodes)
    {
        (void) free(*nodes);
    }

    return status_code;
}

t_return_code LIB_initialize_module(t_module **module, t_logger *logger)
{
    t_return_code status_code = LUKA_UNINITIALIZED;

    *module = calloc(1, sizeof(t_module));
    if (NULL == *module)
    {
        status_code = LUKA_CANT_ALLOC_MEMORY;
        goto l_cleanup;
    }

    (*module)->enums = NULL;
    (*module)->functions = NULL;
    (*module)->structs = NULL;

    RAISE_LUKA_STATUS_ON_ERROR(lib_intialize_nodes(&(*module)->enums, logger),
                               status_code, l_cleanup);
    RAISE_LUKA_STATUS_ON_ERROR(
        lib_intialize_nodes(&(*module)->functions, logger), status_code,
        l_cleanup);
    RAISE_LUKA_STATUS_ON_ERROR(lib_intialize_nodes(&(*module)->structs, logger),
                               status_code, l_cleanup);

    status_code = LUKA_SUCCESS;
    return status_code;
l_cleanup:
    if (NULL != module)
    {
        if (NULL != (*module)->enums)
        {
            (void) lib_free_nodes_vector((*module)->enums, logger);
        }

        if (NULL != (*module)->functions)
        {
            (void) lib_free_nodes_vector((*module)->functions, logger);
        }

        if (NULL != (*module)->structs)
        {
            (void) lib_free_nodes_vector((*module)->structs, logger);
        }
    }
    return status_code;
}

void LIB_free_module(t_module *module, t_logger *logger)
{
    if (NULL != module->functions)
    {
        (void) lib_free_nodes_vector(module->functions, logger);
    }

    if (NULL != module->structs)
    {
        (void) lib_free_nodes_vector(module->structs, logger);
    }

    if (NULL != module->enums)
    {
        (void) lib_free_nodes_vector(module->enums, logger);
    }

    (void) free(module);
    module = NULL;
}

char *LIB_stringify(const char *source, size_t source_length, t_logger *logger)
{
    size_t i = 0;
    size_t char_count = source_length;
    size_t off = 0;
    for (i = 0; i < source_length; ++i)
    {
        switch (source[i])
        {
            case '\n':
            case '\t':
            case '\\':
            case '\"':
                ++char_count;
                break;
            default:
                break;
        }
    }

    ++i;

    char *str = calloc(sizeof(char), char_count + 1);
    if (NULL == str)
    {
        (void) LOGGER_log(
            logger, L_ERROR,
            "Couldn't allocate memory for string in ast_stringify.\n");
        return NULL;
    }

    for (i = 0; i < source_length && i + off < char_count; ++i)
    {
        switch (source[i])
        {
            case '\n':
                str[i + off] = '\\';
                str[i + off + 1] = 'n';
                ++off;
                break;
            case '\t':
                str[i + off] = '\\';
                str[i + off + 1] = 't';
                ++off;
                break;
            case '\\':
                str[i + off] = '\\';
                str[i + off + 1] = '\\';
                ++off;
                break;
            case '\"':
                str[i + off] = '\\';
                str[i + off + 1] = '"';
                ++off;
                break;
            default:
                str[i + off] = source[i];
        }
    }

    str[char_count] = '\0';
    return str;
}
