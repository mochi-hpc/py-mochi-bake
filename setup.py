from distutils.core import setup
from distutils.extension import Extension
from distutils.sysconfig import get_config_vars
import os
import os.path
import sys

(opt,) = get_config_vars('OPT')
os.environ['OPT'] = " ".join(
		    flag for flag in opt.split() if flag != '-Wstrict-prototypes'
		)

try:
    import numpy
    has_numpy = 1
except ImportError:
    has_numpy = 0

if has_numpy == 1:
    client_libs=['boost_python','margo','bake-client', 'boost_numpy']
else:
    client_libs=['boost_python','margo','bake-client']

pybake_server_module = Extension('_pybakeserver', ["pybake/src/server.cpp"],
		           libraries=['boost_python','margo','bake-server'],
                   include_dirs=['.'],
                   depends=[])

pybake_client_module = Extension('_pybakeclient', ["pybake/src/client.cpp"],
		           libraries=client_libs,
                   include_dirs=['.'],
                   depends=[],
                   define_macros=[('HAS_NUMPY', has_numpy)])

pybake_target_module = Extension('_pybaketarget', ["pybake/src/target.cpp"],
                    libraries=['boost_python', 'uuid' ],
                    include_dirs=['.'],
                    depends=[])

setup(name='pybake',
      version='0.1',
      author='Matthieu Dorier',
      description="""Python binding for BAKE""",      
      ext_modules=[ pybake_server_module, pybake_client_module, pybake_target_module ],
      packages=['pybake']
     )
