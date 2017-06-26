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

    public static void unpackNoDoc(Context c, String targetDir) {

        try {
            Log.i("OpenCPN", "assetbridge unpackNoDoc target " + targetDir);


             // now we need the assetmanager
            AssetManager am = c.getAssets();
            String[] assets = am.list("files");

            // iterate on the files...
            for(String asset : assets) {
                if(asset.startsWith("doc")){
                    Log.i("OpenCPN", "assetbridge unpackNoDoc skipping: " + asset);
                }
                else{
                   //Log.i("OpenCPN", "assetbridge asset: " + asset);
                    copyAssetItem(am, "files/"+asset, targetDir + "/" + asset);
                }
            }


        } catch (IOException e) {
            Log.e("OpenCPN", "Can't unpack assets from APK", e);
        }
    }


    public static void unpackDocOnly(AssetManager am, String targetDir) {

        try {
            Log.i("OpenCPN", "assetbridge unpackDocOnly target " + targetDir);


             String[] assets = am.list("files");

            // iterate on the files...
            for(String asset : assets) {
                if(asset.startsWith("doc")){
                    copyAssetItem(am, "files/"+asset, targetDir + "/" + asset);
                }
                else{
                    Log.i("OpenCPN", "assetbridge unpackDocOnly skipping: " + asset);
                }
            }


        } catch (IOException e) {
            Log.e("OpenCPN", "Can't unpack assets from APK", e);
        }
    }


    // unpack
    public static void unpack(Context c, String targetDir) {

        try {

            // now we need the assetmanager
            AssetManager am = c.getAssets();
            String[] assets = am.list("files");

            // iterate on the files...
            for(String asset : assets) {
                //Log.i("OpenCPN", "assetbridge asset: " + asset);
                copyAssetItem(am, "files/"+asset, targetDir + "/" + asset);
            }


        } catch (IOException e) {
            Log.e("OpenCPN", "Can't unpack assets from APK", e);
        }

    }


    public static void copyAssetItem(AssetManager am, String src, String dest)
    	throws IOException{

        //Log.i("OpenCPN", "assetbridge copyAssetItem " + src + " " + dest);

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

            Log.i("OpenCPN", "assetbridge copying dir " + src + " to: " + dest);

            // If the directory doesn't yet exist, create it
            if( !destfh.exists() ){
                destfh.mkdirs();
            }
            if( !destfh.exists() ){
                Log.i("OpenCPN", "assetbridge cannot create directory: " + dest);
            }


            // list the assets in the source directory...
            String assets[] = am.list(src);

            // and copy them all using same.
            for(String asset : assets) {
                copyAssetItem(am, src + "/" + asset, dest + "/" + asset);
            }

    	} else {

            //Log.i("OpenCPN", "assetbridge copying file " + dest);
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
