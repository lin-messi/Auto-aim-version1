from setuptools import find_packages
from setuptools import setup

setup(
    name='rm_interfaces',
    version='1.0.0',
    packages=find_packages(
        include=('rm_interfaces', 'rm_interfaces.*')),
)
