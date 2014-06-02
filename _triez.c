/*

 triez - Fast, pure C, succinct trie

 Sumer Cip 2014

*/

#include "config.h"
#include "trie.h"

// globals
static PyObject *TriezError;

// defines
#ifdef IS_PEP393_AVAILABLE
#define TriezUnicode PyUnicode_1BYTE_DATA
Py_ssize_t TriezUnicode_Len(PyObject *o) {
    // With PEP393, the internal string can be either UCS1, UCS2 or UCS4.
    // However, we use UCS1 internally in our trie as a char type. We cannot 
    // make this dynamic as TRIE_CHAR param is a compile time attribute. So we
    // need to somehow detect how many UCS1 chars we have for the given Unicode
    // string.
    int kind;
    Py_ssize_t len;
    
    kind = PyUnicode_KIND(o);
    len = PyUnicode_GET_LENGTH(o);
    if (kind == PyUnicode_2BYTE_KIND) {
        //printf("2kind\r\n");
        len *= 2;
    } else if (kind == PyUnicode_4BYTE_KIND) {
        //printf("4kind\r\n");
        len *= 4;
    } 
    return len;
}
#else
#define TriezUnicode PyUnicode_AS_UNICODE
#define TriezUnicode_Len PyUnicode_GET_SIZE
#endif

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

// module custom types
typedef struct {
    PyObject_HEAD
    trie_t *ptrie;
} TrieObject;

static PyObject* Trie_mem_usage(TrieObject* self)
{
    return Py_BuildValue("l", trie_mem_usage(self->ptrie));
}

static PyObject* Trie_node_count(TrieObject* self)
{
    return Py_BuildValue("l", self->ptrie->node_count);
}

static PyMethodDef Trie_methods[] = {
    {"mem_usage", (PyCFunction)Trie_mem_usage, METH_NOARGS, 
        "Memory usage of the trie. Used for debugging purposes."},
    {"node_count", (PyCFunction)Trie_node_count, METH_NOARGS, 
        "Node count of the trie. Used for debugging purposes."},
    {NULL}  /* Sentinel */
};

static void Trie_dealloc(TrieObject* self)
{
    //printf("destructor\r\n");
    trie_destroy(self->ptrie);
}

static PyObject *Trie_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    TrieObject *self;
    
    self = (TrieObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->ptrie = trie_create();
        if (!self->ptrie) {
            return NULL;
        }
    }

    return (PyObject *)self;
}

int _IsValid_Unicode(PyObject *s)
{
    if (!PyUnicode_Check(s)) {
        return 0;
    }
    
#ifdef IS_PEP393_AVAILABLE
    if (PyUnicode_READY(s) == -1) {
        return 0;
    }
#endif
    
    return 1;
}

trie_key_t _PyUnicode_AS_TKEY(PyObject *s) 
{   
    //int i;
    trie_key_t k;
    
    k.s = TriezUnicode(s);
    k.len = TriezUnicode_Len(s);
    
    /*
    //TODO: Move to Triez_Unicode_debug_print()
    printf("k->len:%d\r\n", k->len);
    for(i=0;i<k->len;i++) {
        printf("chr:%d\r\n", k->s[i]);
    }*/
    
    return k;
}

/* Return 1 if `key` is in trie `op`, 0 if not, and -1 on error. */
int Trie_contains(PyObject *op, PyObject *key)
{
    trie_key_t k;
    TrieObject *mp = (TrieObject *)op;
    
    if (!_IsValid_Unicode(key)) {
        return 0; // donot return exception here.
    }
    
    k = _PyUnicode_AS_TKEY(key);
    if(!trie_search(mp->ptrie, &k)) {
        return 0;
    }
    
    return 1;
}

static Py_ssize_t trie_length(TrieObject *mp)
{
    return mp->ptrie->item_count;
}

static PyObject *trie_subscript(TrieObject *mp, PyObject *key)
{
    trie_key_t k;
    PyObject *v;
    trie_node_t *w;
    
    if (!_IsValid_Unicode(key)) {
        PyErr_SetString(TriezError, "key must be a valid unicode string.");
        return NULL;
    }
    
    k = _PyUnicode_AS_TKEY(key);
    w = trie_search(mp->ptrie, &k);
    if (!w) {
        PyErr_SetObject(PyExc_KeyError, key);
        return NULL;
    }
    
    v = (PyObject *)w->value;
    Py_INCREF(v);
    
    return v;
}

/* Return 0 on success, and -1 on error. */
static int trie_ass_sub(TrieObject *mp, PyObject *key, PyObject *val)
{
    trie_key_t k;
    
    if (!_IsValid_Unicode(key)) {
        PyErr_SetString(TriezError, "key must be a valid unicode string.");
        return -1;
    }
    
    k = _PyUnicode_AS_TKEY(key);
    if (val == NULL) {
        if (!trie_del_fast(mp->ptrie, &k)) {
            PyErr_SetObject(PyExc_KeyError, key);
            return -1;
        }
    } else {
        Py_INCREF(val);
        if(!trie_add(mp->ptrie, &k, (uintptr_t)val)) {
            PyErr_SetString(TriezError, "key cannot be added.");
            return -1;
        }
    }
    return 0;
}

/* Hack to implement "key in trie" */
static PySequenceMethods Trie_as_sequence = {
    0,                              /* sq_length */
    0,                              /* sq_concat */
    0,                              /* sq_repeat */
    0,                              /* sq_item */
    0,                              /* sq_slice */
    0,                              /* sq_ass_item */
    0,                              /* sq_ass_slice */
    Trie_contains,                  /* sq_contains */
    0,                              /* sq_inplace_concat */
    0,                              /* sq_inplace_repeat */
};

static PyMappingMethods Trie_as_mapping = {
    (lenfunc)trie_length,           /*mp_length*/
    (binaryfunc)trie_subscript,     /*mp_subscript*/
    (objobjargproc)trie_ass_sub,    /*mp_ass_subscript*/
};

static PyTypeObject TrieType = {
#ifdef IS_PY3K
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
#endif
    "_triez.Trie",                  /* tp_name */
    sizeof(TrieObject),             /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)Trie_dealloc,       /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    &Trie_as_sequence,              /* tp_as_sequence */
    &Trie_as_mapping,               /* tp_as_mapping */
    PyObject_HashNotImplemented,    /* tp_hash  */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    "Trie objects",                 /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    Trie_methods,                   /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
    Trie_new,                       /* tp_new */
};

static PyMethodDef Triez_methods[] = {
    {NULL, NULL}      /* sentinel */
};

#ifdef IS_PY3K
PyDoc_STRVAR(Triez__doc__, "Fast, pure C, succinct trie");
static struct PyModuleDef Triez_module = {
    PyModuleDef_HEAD_INIT,
    "Triez",
    Triez__doc__,
    -1,
    Triez_methods,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

PyMODINIT_FUNC
#ifdef IS_PY3K
PyInit_Triez(void)
#else
initTriez(void)
#endif
{
    PyObject *m;
    
    if (PyType_Ready(&TrieType) < 0) {
#ifdef IS_PY3K
        return NULL;
#else
        return;
#endif
    }
    
#ifdef IS_PY3K
    m = PyModule_Create(&Triez_module);
    if (m == NULL)
        return NULL;
#else
    m = Py_InitModule("Triez",  Triez_methods);
    if (m == NULL)
        return;
#endif
    
    Py_INCREF(&TrieType);
    PyModule_AddObject(m, "Trie", (PyObject *)&TrieType);
    
    TriezError = PyErr_NewException("Triez.Error", NULL, NULL);
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
