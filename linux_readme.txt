3rd party Libs & Packages
-------------------------
1. GSL
   Under CentOS the following packages
      gsl
      gsl-devel
   available from repo "base"

2. Qwt
   Packages:
     qwt
     qwt-devel
   from fedore EPEL repo.

3. HDF5
    Packages:
      hdf5
      hdf5-devel
    from fedore EPEL repo.

4. muParser
Packages :
  muParser
  muParser-devel
from fedora EPEL repo.

5. Atlas Blas
Package atlas.i686
  needs a symlink to work
  sudo ln -s /usr/lib/atlas/libcblas.so.3.0 /usr/lib/libcblas.so
  sudo ln -s /usr/lib/atlas/libatlas.so.3.0 /usr/lib/libatlas.so

CentOS/RedHat notes
-------------------
Find packages with
>yum list name*
Install
>yum install name
List installed files
>rpm -ql --filesbypkg name
