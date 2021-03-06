from distutils.core import setup
from distutils.extension import Extension
from distutils.sysconfig import get_config_vars
import pybind11
import pkgconfig
import os
import os.path
import sys

(opt,) = get_config_vars('OPT')
os.environ['OPT'] = " ".join(
		    flag for flag in opt.split() if flag != '-Wstrict-prototypes'
		)
# Find out if Numpy is present
try:
    import numpy
    has_numpy = 1
except ImportError:
    has_numpy = 0

# Find out the dependencies using pkgconfig 
# For client...
bake_client = pkgconfig.parse('bake-client')
client_libraries = bake_client['libraries']
client_library_dirs = bake_client['library_dirs']
client_include_dirs = bake_client['include_dirs']
client_include_dirs.append(".")
client_include_dirs.append(pybind11.get_include())
# For server...
bake_server = pkgconfig.parse('bake-server')
server_libraries = bake_server['libraries']
server_library_dirs = bake_server['library_dirs']
server_include_dirs = bake_server['include_dirs']
server_include_dirs.append(".")
server_include_dirs.append(pybind11.get_include())
# For target...
uuid = pkgconfig.parse('uuid')
target_libraries = uuid['libraries'] + bake_client['libraries']
target_library_dirs = uuid['library_dirs'] + bake_client['library_dirs']
target_include_dirs = uuid['include_dirs'] + bake_client['include_dirs']
target_include_dirs.append('.')
target_include_dirs.append(pybind11.get_include())
pk = pkgconfig.parse('bake-server')
target_include_dirs.extend(pk['include_dirs'])

pybake_server_module = Extension('_pybakeserver', ["pybake/src/server.cpp"],
		           libraries=server_libraries,
                   library_dirs=server_library_dirs,
                   include_dirs=server_include_dirs,
                   extra_compile_args=['-std=c++11'],
                   depends=["pybake/src/server.cpp"])

pybake_client_module = Extension('_pybakeclient', ["pybake/src/client.cpp"],
		           libraries=client_libraries,
                   library_dirs=client_library_dirs,
                   include_dirs=client_include_dirs,
                   extra_compile_args=['-std=c++11'],
                   depends=["pybake/src/client.cpp"],
                   define_macros=[('HAS_NUMPY', has_numpy)])

pybake_target_module = Extension('_pybaketarget', ["pybake/src/target.cpp"],
                    libraries=target_libraries,
                    library_dirs=target_library_dirs,
                    include_dirs=target_include_dirs,
                    extra_compile_args=['-std=c++11'],
                    depends=["pybake/src/target.cpp"])

setup(name='pybake',
      version='0.2.1',
      author='Matthieu Dorier',
      description="""Python binding for BAKE""",      
      ext_modules=[ pybake_server_module, pybake_client_module, pybake_target_module ],
      packages=['pybake']
     )
