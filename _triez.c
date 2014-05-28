/*

 triez - Fast, pure C, succinct trie

 Sumer Cip 2014

*/

#include "config.h"
#include "trie.h"

// globals
static PyObject *TriezError;

// defines

// forwards

// module functions

int _initialize(void)
{
    return 1;
}

void print_PTR(void *ptr)
{
    printf("ptr:%p\r\n", ptr);
}

void *_get_trie_from_args(PyObject *args)
{
    PyObject *caps;
    void *result;
    
    if (!PyArg_ParseTuple(args, "O", &caps)) {
        goto err;
    }
    
    result = PyCapsule_GetPointer(caps, NULL);
    if(!result) {
        goto err;
    }
    return result;
err:
    PyErr_SetString(TriezError, "trie cannot be retrieved from args.");
    return NULL;
}

static PyObject* triez_trie_create(PyObject *self, PyObject *args)
{
    trie_t *tr;
    
    tr = trie_create();
    //print_PTR(tr);
    if (!tr) {
        PyErr_SetString(TriezError, "trie cannot be initialized.");
        return NULL;
    }
    
    return PyCapsule_New(tr, NULL, NULL);
}



static PyObject* triez_trie_destroy(PyObject *self, PyObject *args)
{
    trie_t *tr;
    
    tr = (trie_t *)_get_trie_from_args(args); 
    if(!tr) {
        return NULL;
    }
    
    trie_destroy(tr);
    
    Py_RETURN_NONE;
}

static PyObject* triez_trie_mem_usage(PyObject *self, PyObject *args)
{
    trie_t *tr;
    
    tr = (trie_t *)_get_trie_from_args(args); 
    if(!tr) {
        return NULL;
    }
    
    return Py_BuildValue("l", trie_mem_usage(tr));
}

static PyObject* triez_trie_node_count(PyObject *self, PyObject *args)
{
    trie_t *tr;
    
    tr = (trie_t *)_get_trie_from_args(args); 
    if(!tr) {
        return NULL;
    }
    
    return Py_BuildValue("l", tr->node_count);
}


static PyObject* triez_trie_search(PyObject *self, PyObject *args)
{
    trie_t *tr;
    trie_key_t k;
    int key_size;
    trie_node_t *nd;
    Py_UNICODE *key;
    PyObject *caps;
    
    if (!PyArg_ParseTuple(args, "Ou#", &caps, &key, &key_size)) {
        PyErr_SetString(TriezError, "invalid argument list. key must be a unicode string.");
        return NULL;
    }
    
    tr = (trie_t *)PyCapsule_GetPointer(caps, NULL);
    if (!tr) {
        PyErr_SetString(TriezError, "invalid trie.");
        return NULL;
    }
    
    k.s = key;
    k.len = key_size;
    nd = trie_search(tr, &k);
    if (!nd) {
        Py_RETURN_NONE;
    }
    
    Py_INCREF((PyObject *)nd->value);
    return (PyObject *)nd->value;
}

static PyObject* triez_trie_add(PyObject *self, PyObject *args)
{
    trie_t *tr;
    trie_key_t k;
    int key_size;
    Py_UNICODE *key,*val;
    PyObject *caps;
    
    if (!PyArg_ParseTuple(args, "Ou#O", &caps, &key, &key_size, &val)) {
        PyErr_SetString(TriezError, "invalid argument list. key must be a unicode string.");
        return NULL;
    }
    
    tr = (trie_t *)PyCapsule_GetPointer(caps, NULL);
    if (!tr) {
        PyErr_SetString(TriezError, "invalid trie.");
        return NULL;
    }
    
    k.s = key;
    k.len = key_size;
    Py_INCREF(val);
    if (!trie_add(tr, &k, (uintptr_t)val))
    {
        PyErr_SetString(TriezError, "key cannot be added.");
        return NULL;
    }
    
    Py_RETURN_TRUE;
}

static PyObject* triez_trie_delete(PyObject *self, PyObject *args)
{
    trie_t *tr;
    trie_key_t k;
    Py_UNICODE *key;
    PyObject *caps;
    int key_size;
    
    if (!PyArg_ParseTuple(args, "Ou#", &caps, &key, &key_size)) {
        PyErr_SetString(TriezError, "invalid argument list. key must be a unicode string.");
        return NULL;
    }
    
    tr = (trie_t *)PyCapsule_GetPointer(caps, NULL);
    if (!tr) {
        PyErr_SetString(TriezError, "invalid trie.");
        return NULL;
    }
    
    k.s = key;
    k.len = key_size;
    if (!trie_del_fast(tr, &k))
    {
        Py_RETURN_FALSE;
    }
    
    Py_RETURN_TRUE;
}

static PyMethodDef _triez_methods[] = {
    // trie methods 
    {"trie_create", triez_trie_create, METH_VARARGS, NULL},
    {"trie_destroy", triez_trie_destroy, METH_VARARGS, NULL},
    {"trie_mem_usage", triez_trie_mem_usage, METH_VARARGS, NULL},
    {"trie_node_count", triez_trie_node_count, METH_VARARGS, NULL},
    {"trie_search", triez_trie_search, METH_VARARGS, NULL},
    {"trie_add", triez_trie_add, METH_VARARGS, NULL},
    {"trie_delete", triez_trie_delete, METH_VARARGS, NULL},
    {NULL, NULL}      /* sentinel */
};

#ifdef IS_PY3K
PyDoc_STRVAR(_triez__doc__, "Fast, pure C, succinct trie");
static struct PyModuleDef _triez_module = {
    PyModuleDef_HEAD_INIT,
    "_triez",
    _triez__doc__,
    -1,
    _triez_methods,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

PyMODINIT_FUNC
#ifdef IS_PY3K
PyInit__triez(void)
#else
init_triez(void)
#endif
{
    PyObject *m;    

#ifdef IS_PY3K
    m = PyModule_Create(&_triez_module);
    if (m == NULL)
        return NULL;
#else
    m = Py_InitModule("_triez",  _triez_methods);
    if (m == NULL)
        return;
#endif

    TriezError = PyErr_NewException("_triez.Error", NULL, NULL);
    PyDict_SetItemString(PyModule_GetDict(m), "Error", TriezError);
    
    if (!_initialize()) {
        PyErr_SetString(TriezError, "triez module cannot be initialized.");
#ifdef IS_PY3K
        return NULL;
#else
        return;
#endif
    }

#ifdef IS_PY3K
    return m;
#endif
}
