package com.arieslabs.assetbridge;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import android.content.pm.ApplicationInfo;


public class Assetbridge {
    static {
        System.loadLibrary("assetbridge");
    }


    // unpack
    public static void unpack(Context c) {

        try {
            String targetDir = "";

            // first let's get the target directory

            ApplicationInfo ai = c.getApplicationInfo();
            if((ai.flags & ApplicationInfo.FLAG_EXTERNAL_STORAGE) ==  ApplicationInfo.FLAG_EXTERNAL_STORAGE)
                targetDir = c.getExternalFilesDir(null).getPath();
            else
                targetDir = c.getFilesDir().getPath();

            Log.i("OpenCPN", "assetbridge target " + targetDir);


            // Sometimes, due to an app startup race condition, the getFilesDir() does not yet exist, or is inaccessible.
            //  So, in this case, we hardcode/set the targetDIR directly.
            //  Note that this method WILL PROBABLY NOT WORK if the app is moved to the sdCard.  But it does not always happen, so...
            // See https://code.google.com/p/android/issues/detail?id=8886

            if(targetDir.isEmpty())
                targetDir = "/data/data/org.opencpn.opencpn/files";

            // now we need the assetmanager
            AssetManager am = c.getAssets();
            String[] assets = am.list("files");

            // iterate on the files...
            for(String asset : assets) {
                Log.i("OpenCPN", "assetbridge asset: " + asset);
                copyAssetItem(am, "files/"+asset, targetDir + "/" + asset);
            }


        } catch (IOException e) {
            Log.e("OpenCPN", "Can't unpack assets from APK", e);
        }

    }


    public static void copyAssetItem(AssetManager am, String src, String dest)
    	throws IOException{

        Log.i("OpenCPN", "assetbridge copyAssetItem " + src + " " + dest);

        InputStream srcIS = null;
        File destfh;

        // this is the only way we can tell if this is a file or a
        // folder - we have to open the asset, and if the open fails,
        // it's a folder...
        boolean isDir = false;
        try {
            srcIS = am.open(src);
        } catch (FileNotFoundException e) {
            isDir = true;
        }

        // either way, we'll use the dest as a File
        destfh = new File(dest);

        // and now, depending on ..
    	if(isDir) {

            Log.i("OpenCPN", "assetbridge copying dir " + dest);

            // If the directory doesn't yet exist, create it
            if( !destfh.exists() ){
                destfh.mkdir();
            }

            // list the assets in the directory...
            String assets[] = am.list(src);

            // and copy them all using same.
            for(String asset : assets) {
                copyAssetItem(am, src + "/" + asset, dest + "/" + asset);
            }

    	} else {

            Log.i("OpenCPN", "assetbridge copying file " + dest);
            int count, buffer_len = 2048;
            byte[] data = new byte[buffer_len];

            // copy the file from the assets subsystem to the filesystem
            FileOutputStream destOS = new FileOutputStream(destfh);

    	    //copy the file content in bytes
            while( (count = srcIS.read(data, 0, buffer_len)) != -1) {
                destOS.write(data, 0, count);
            }

            // and close the two files
            srcIS.close();
            destOS.close();

    	}
    }


    // the native method to set the environment variable
    public static native void setassetdir(String s);
}
