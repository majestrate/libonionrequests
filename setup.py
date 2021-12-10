from setuptools import setup
from glob import glob

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "0.0.1"

# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension(
        "pyonionreq",
        sorted([
            *glob("onionreq/*.cpp"),
            *glob("pybind/*.cpp")
        ]),
        include_dirs=['.'],
        libraries=['oxenmq', 'sodium', 'fmt', 'spdlog', 'crypto'],
        cxx_std=17,
    ),
]

setup(
    name="pyonionreq",
    version=__version__,
    author="Jason Rhinelander",
    author_email="jason@oxen.io",
    url="https://github.com/oxen-io/libonionrequests",
    description="Python wrapper for oxen onion request utilities",
    long_description="",
    ext_modules=ext_modules,
    zip_safe=False,
)

