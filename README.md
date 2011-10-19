# usersv

This program is designed to work with runit by Gerrit Pape.
	http://smarden.org/runit/

Usersv creates a seperate runit service manager for each user in the "svusers"
group as that user, in the ~/.sv directory.  This means that each user can have
their own services that start with the system, but run as that user.  If the
runit sv tool is in a globally accessable place then users can manage their own
services.

If a user is added to the "svusers" group after the service is started, their
service manager is started automatically, provided the have a .sv directory in
their home directory.

# Security

I believe that this program is pretty darn secure, given what it does.  Only
users you trust should be placed within the svusers group, but the runsv
service is run as the given user, so the damage they can do is still minimal,
and depends primarily on your system's configuration.

# Building & Installing

The program is one source file, it doesn't need much configuration.  You can
pretty much just run make.

If you want to change the service directory, or the group you can add variables
to the make command line:

	# this sets the group to "users" and the service dir will be ~/services
    make GROUP=users SVDIR=services
	# this installs usersv into /usr/sbin
	make install

If you don't want to install to /usr/sbin, you can set PREFIX on the make
command line.  I.e:

	# this installs usersv into /usr/local/sbin
	make PREFIX=/usr/local install

You should then setup a new runit service that runs usersv.  It must run as
root, but has no configuration or command line parameters.

## Configuration

Sure, usersv doesn't have any configuration files, but you still need to add the
svusers group to your system, and probably put some users in that group.  It's
ok to add the group after the program starts, and add users to the svusers group
after usersv is running.  Usersv will check for new users activate their service
manager every 30 seconds.
