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

NI DAQ notes
------------

NI Linux DRIVERS

1) NI-DAQmx 8.0.2 (2011)
supports 6221 & 6602
openSUSE 11.1-11.3 (kernel 2.6.x)
RHEL 4 & 5

2) NI-DAQmx Base (Nov 2014)
supports only 6221
openSUSE 12.3 (kernel 3.7.10) and 13.1 (kernel 3.11.6)
Scientific Linux 6.x
RHEL 6 (kernel 2.6.x)

CentOS/RedHat notes
-------------------
Find packages with
>yum list name*

Install
>yum install name

List installed files
>rpm -ql --filesbypkg name
