package org.opencpn;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.TextView;
import android.content.Intent;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;
import android.app.IntentService;
import android.os.ResultReceiver;
import java.io.BufferedInputStream;
import java.io.OutputStream;
import java.net.URLConnection;
import java.io.IOException;
import android.net.Uri;
import android.support.v4.provider.DocumentFile;
import android.preference.PreferenceManager;
import android.content.SharedPreferences;
import java.util.ArrayList;
import java.util.List;

public class DownloadService extends IntentService {
    public static final int UPDATE_PROGRESS = 8344;
    public static final int DOWNLOAD_DONE = 8345;
    public DownloadService() {
        super("DownloadService");
//        Log.i("OpenCPN", "DownloadService ctor ");
    }
    @Override
    protected void onHandleIntent(Intent intent) {
        String urlToDownload = intent.getStringExtra("url");
        String fileDestination = intent.getStringExtra("file");
        boolean bUseDocFile = intent.getBooleanExtra("useDocFile", false);

        Uri fURI = Uri.parse(fileDestination);
        String file = "";
        try{
            file = fURI.getPath();
        }catch (Exception e) {
        }

        ResultReceiver receiver = (ResultReceiver) intent.getParcelableExtra("receiver");
        Log.i("OpenCPN", "onHandleIntent " + fileDestination + " " + file );

        OutputStream outputStream = null;

        if(bUseDocFile){
            Log.i("OpenCPN", "onHandleIntent : using DocumentFile interface for: " + file );

            File destFile = new File(file);
            DocumentFile dfile = getDocumentFile(destFile, false, true);

            if(null == dfile)
                Log.i("OpenCPN", "onHandleIntent : dfile is NULL" );


            try{
                outputStream = getContentResolver().openOutputStream(dfile.getUri());
            }
            catch(Exception e){
                Log.i("OpenCPN", "onHandleIntent exceptionA: " + file);
            }


        }
        else{
            try{
                outputStream = new FileOutputStream(file);
            }
            catch(Exception e){
                Log.i("OpenCPN", "onHandleIntent exceptionB: " + file);
            }
        }








        long total = 0;
        int fileLength = -1;

        if(null != outputStream){
        try {
            URL url = new URL(urlToDownload);
            URLConnection connection = url.openConnection();
            connection.connect();

            // this will be useful so that you can show a typical 0-100% progress bar
            fileLength = connection.getContentLength();

            // download the file
            InputStream input = new BufferedInputStream(connection.getInputStream());


            byte data[] = new byte[1024];
            int count;
            while ((count = input.read(data)) != -1) {
                total += count;
                // publishing the progress....
                Bundle resultData = new Bundle();

                int progress = (int) (total * 100 / fileLength);
                resultData.putInt("progress" , progress);
                resultData.putInt("sofar" , (int)total);
                resultData.putInt("filelength" , fileLength);

//                Log.i("OpenCPN", "RECEIVER SEND " + total + " " + fileLength + " " + progress);
                receiver.send(UPDATE_PROGRESS, resultData);
                outputStream.write(data, 0, count);
            }

            outputStream.flush();
            outputStream.close();
            input.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        }

        Bundle resultData = new Bundle();
        resultData.putInt("progress" ,100);
        resultData.putInt("sofar" , (int)total);
        resultData.putInt("filelength" , fileLength);
        receiver.send(DOWNLOAD_DONE, resultData);
    }

    /**
        * Get a list of external SD card paths. (Kitkat or higher.)
        *
        * @return A list of external SD card paths.
        */
       private String[] getExtSdCardPaths() {
           List<String> paths = new ArrayList<String>();



               for (File file : this.getExternalFilesDirs("external")) {
                   if (file != null && !file.equals(this.getExternalFilesDir("external"))) {
                       int index = file.getAbsolutePath().lastIndexOf("/Android/data");
                       if (index < 0) {
                           //Log.w(Application.TAG, "Unexpected external file dir: " + file.getAbsolutePath());
                       } else {
                           String path = file.getAbsolutePath().substring(0, index);
                           try {
                               path = new File(path).getCanonicalPath();
                           } catch (IOException e) {
                               // Keep non-canonical path.
                           }
                           paths.add(path);
                       }
                   }
               }

           return paths.toArray(new String[paths.size()]);
       }


    public String getExtSdCardFolder(final File file) {
        String[] extSdPaths = getExtSdCardPaths();
        try {
            for (int i = 0; i < extSdPaths.length; i++) {
                if (file.getCanonicalPath().startsWith(extSdPaths[i])) {
                   return extSdPaths[i];
                }
           }
        }
        catch (IOException e) {
           return null;
        }

        return null;
   }

    public DocumentFile getDocumentFile(final File file, final boolean isDirectory, boolean bCreate) {
        String baseFolder = getExtSdCardFolder(file);

        if (baseFolder == null) {
            return null;
        }

        String relativePath = null;
        try {
            String fullPath = file.getCanonicalPath();
            if(fullPath.length() > baseFolder.length())
                relativePath = fullPath.substring(baseFolder.length() + 1);
            else
            relativePath = "";
            }
        catch (IOException e) {
            return null;
        }

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        String sUri = preferences.getString("SDURI", "");
        if(sUri.isEmpty())
            return null;

        Uri ltreeUri = Uri.parse(sUri);

        if (ltreeUri == null) {
            return null;
        }


        // start with root of SD card and then parse through document tree.
        DocumentFile document = DocumentFile.fromTreeUri(this, ltreeUri);

        if(relativePath.isEmpty())
            return document;

        try {
            String[] parts = relativePath.split("\\/");
            for (int i = 0; i < parts.length; i++) {
                DocumentFile nextDocument = document.findFile(parts[i]);

                if (nextDocument == null) {
                    if(!bCreate)
                        return null;
                    if ((i < parts.length - 1) || isDirectory) {
                        nextDocument = document.createDirectory(parts[i]);
                    } else {
                        nextDocument = document.createFile("image", parts[i]);
                    }
                }
                document = nextDocument;
                }
            }catch(Exception e){
                int yyp = 0;
            }

        return document;
        }



}































