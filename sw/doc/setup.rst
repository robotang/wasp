Building the Software
=====================

In order to build the system the following software must be installed on your 
computer.

Building the Documentation (Optional)
-------------------------------------
- python-sphinx > 0.6 
- doxygen

If you are using Ubuntu Jaunty, you may :xref:`wasp-download` sphinx from the :xref:`wasp`
homepage. Ubunty Karmic users, or users of other distributions should install
sphinx from your package manager.

Building the Onboard Software
-----------------------------
- newlib-arm_1.13_all.deb
- lpc21isp_0.1_i386.deb
- gcc-arm_3.4.4_i386.deb
- binutils-arm_2.16.1_i386.deb

You may :xref:`wasp-download` these packages from the :xref:`wasp` homepage.

.. _groundstation-setup:

Building the Groundstation
--------------------------
- Install the dependencies

::

    sudo apt-get install python-gtk2 python-serial python-gtkglext1 python-opengl

- Install osm-gps-map

The groundstation depends on :xref:`osm-gps-map`. It works with the version shipped
in Ubuntu lucid, so it can be installed by simply typing::

    sudo apt-get install python-osmgpsmap

If you wish to use a later verion, :xref:`wasp` also supports recent stable releases
of :xref:`osm-gps-map`. In this case, you should follow the installation instruction
on the :xref:`osm-gps-map` homepage.

One thing to note; you do not have to install osm-gps-map to /usr if you do not wish to.
To run the groundstation against an uninstalled copy of osm-gps-map remember to
set PYTHONPATH environment variable. For example::

    PYTHONPATH=/path/to/osm-gps-map/python/.libs ./groundstation.py

- Check out the code from :xref:`wasp`.

::

    git clone git://github.com/nzjrs/wasp.git

- Update any git submodules

::

    git submodule init
    git submodule update

- Checkout the branch tha represents the configuration you are flying

::

    git checkout -b name-of-branch origin/name-of-branch

- Install the bootloader rules (this makes the usb port accessible by non-root users, i.e. those that are members of the plugdev group).

::

    sudo cp ./sw/bootloader/10-wasp-uav-bootloader.rules /etc/udev/rules.d/

- Ensure that the user who is going to program the Autopilot is a member of the ``plugdev`` group.
- Check the groundstation runs

::

    cd sw/groundstation
    ./groundstation.py

- Install the groundstation (optional)
  Using this method, the groundstation can be started from the Applications menu in GNOME, however for developemnt it is recommended to run the groundstation from the command line

::

    cd sw/groundstation
    make install

- Build the onboard code

::

    cd sw/onboard
    make

Programming the Autopilot
-------------------------
The autopilot software is programmed using a built in USB bootloader. To enter programming mode, the autopilot must be powered on with the USB port connected to the computer. 

- Using the command line

::

    cd sw/onboard
    make upload

- Using the Groundstation. To program the autopilot you may select the *UAV -> Program Autopilot* menu option, and select *Program*. Any errors are printed to the console

Other commands can also be applied to change what is build, for example

- make TARGET=target_name (test/test_led for example)
- make ARCH=arch_name
- make clean
- make generated



