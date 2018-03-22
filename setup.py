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

pybake_server_module = Extension('_pybakeserver', ["pybake/src/server.cpp"],
		           libraries=['boost_python','margo','bake-server'],
			   include_dirs=['.'],
			   depends=[])
setup(name='pybake',
      version='0.1',
      author='Matthieu Dorier',
      description="""Python binding for BAKE""",      
      ext_modules=[ pybake_server_module ],
      packages=['pybake']
     )
