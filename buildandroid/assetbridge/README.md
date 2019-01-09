Assetbridge
===========

Assetbridge is a small Java and native-code module that lets you
access the the contents of the assets/ folder in your Android .APK
from NDK modules.

Assetbridge works by copying the assets/ directory structure into a
temporary directory on the filesystem, and then setting an environment
variable to the location of that temporary directory.


Background
----------

Android packages provide a convenient way to package up arbitrary data
with your application using the AssetManager class. Anything you store
in the assets/ folder of an Android project is automatically wrapped
up and included in the .APK file, and you can then access that data
through the Java AssetManager classes.

This works well enough for pure-Java applications. If you're writing a
project with both Java and native components, however, you'll probably
run up against the limitations in the AssetManager API. You might not
want to hard-code the directory structure of your assets/ data, but
have your code read that directory structure at runtime. There might
be native libraries you'd like to use, for example, but can't, because
they have no support for the AssetManager API.

Because of these reasons, the AssetManager interface for native code
can be frustrating to try to work around.

Assetbridge is intended to solve those frustrations. It works around
the limitations in AssetManager in a way that should end up being
largely compatible with your native code. What it does is extract the
entire assets/ tree into a temporary directory on the Android
filesystem, and then it sets an environment variable, *ASSETDIR*,
readable by your native code, to that directory.


How to add Assetbridge to your project
--------------------------------------

Assetbridge comes in two parts: the NDK module and the Java class to
call it. It helps to have some familiarity with the native code build
process, ndk-build.

First, copy the /jni/assetbridge directory to your project's JNI
directory. Then, copy the /src/com/arieslabs/assetbridge directory into
your project's source tree.

Run *ndk-build* to make sure that the module's compiled, and when
that's done, you're ready to add it to the Java side of your project.


How to use Assetbridge
----------------------

Assetbridge consists of a single static method that takes a Context
object and doesn't return anything.

Make sure you import the module at the top of your .java file:

    import com.arieslabs.assetbridge.Assetbridge;

Then, in the main activity of your Android program, in the *onCreate*
method, call the *unpack* method like so:

    Assetbridge.unpack(this);

And that's it! Assetbridge unpacks the assets/ tree into a temporary
directory and sets *ASSETDIR* to the path to that directory. You can
now read the environment variable in your native code module, chdir()
to the temporary directory, and read the files using the standard
POSIX file and directory routines.


Questions? Comments?
--------------------

Please feel free to contact me with questions or comments.

My email address: *steve@arieslabs.com*

My public key: http://stevehavelka.com/steve.pub
