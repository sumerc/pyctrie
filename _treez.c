/*

 treez - Fast, pure C tree structures library

 Sumer Cip 2014

*/

#include "config.h"
#include "trie.h"

// globals
static PyObject *TreezError;

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

static PyObject* treez_trie_create(PyObject *self, PyObject *args)
{
    trie_t *tr;
    
    tr = trie_create();
    print_PTR(tr);
    if (!tr) {
        PyErr_SetString(TreezError, "trie cannot be initialized.");
    }
    
    return PyCapsule_New(tr, NULL, NULL);
}

static PyObject* treez_trie_destroy(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
}

static PyObject* treez_trie_search(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
}

static PyObject* treez_trie_add(PyObject *self, PyObject *args)
{
    trie_t *tr;
    trie_key_t k;
    PyObject *caps;

    if (!PyArg_ParseTuple(args, "O", &caps)) {
        return NULL;
    }
    
    tr = PyCapsule_GetPointer(caps, NULL);
    if (!tr) {
        return NULL;
    }
    print_PTR(tr);
    k.s = "fff";
    k.len = 3;
    trie_add(tr, &k, 1234);
    print_PTR(tr);
    Py_RETURN_NONE;
}

static PyObject* treez_trie_del(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
}

static PyMethodDef _treez_methods[] = {
    // trie methods 
    {"trie_create", treez_trie_create, METH_VARARGS, NULL},
    {"trie_destroy", treez_trie_destroy, METH_VARARGS, NULL},
    {"trie_search", treez_trie_search, METH_VARARGS, NULL},
    {"trie_add", treez_trie_add, METH_VARARGS, NULL},
    {"trie_del", treez_trie_del, METH_VARARGS, NULL},
    {NULL, NULL}      /* sentinel */
};

#ifdef IS_PY3K
PyDoc_STRVAR(_treez__doc__, "Fast, efficient tree structures");
static struct PyModuleDef _treez_module = {
    PyModuleDef_HEAD_INIT,
    "_treez",
    _treez__doc__,
    -1,
    _treez_methods,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

PyMODINIT_FUNC
#ifdef IS_PY3K
PyInit__treez(void)
#else
init_treez(void)
#endif
{
    PyObject *m;    

#ifdef IS_PY3K
    m = PyModule_Create(&_treez_module);
    if (m == NULL)
        return NULL;
#else
    m = Py_InitModule("_treez",  _treez_methods);
    if (m == NULL)
        return;
#endif

    TreezError = PyErr_NewException("_treez.error", NULL, NULL);
    PyDict_SetItemString(PyModule_GetDict(m), "error", TreezError);

    if (!_initialize()) {
        PyErr_SetString(TreezError, "Treez cannot be initialized.");
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
