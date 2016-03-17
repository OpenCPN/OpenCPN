package org.opencpn;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.io.IOException;
import java.io.FileInputStream;

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

public class downloadGFSCombine extends Activity {

    //initialize our progress dialog/bar
    private ProgressDialog mProgressDialog;
    public static final int DIALOG_DOWNLOAD_PROGRESS = 0;
    public static final int ERROR_NO_INTERNET = 1;
    public static final int ERROR_NO_CONNECTION = 2;
    public static final int ERROR_EXCEPTION = 3;

    public int m_result = RESULT_OK;
    public ArrayList<String> URLList;
    public ArrayList<String> fileNameList;

    private static final int BUFFER_SIZE = 1024;
    public int nBlock;

    //initialize root directory
    //File rootDir = Environment.getExternalStorageDirectory();
    File rootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);

    //defining file name and url
    public String fileName = "";
    public String fileURL = "";
    public String destinationFileName = "";
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        //setting some display
     //   setContentView(R.layout.main);
     //   TextView tv = new TextView(this);
     //   tv.setText("Android Download File With Progress Bar");

        //making sure the download directory exists
        //checkAndCreateDirectory("/my_downloads");

        Bundle extras = getIntent().getExtras();
        destinationFileName = extras.getString("GRIB_dest_file");
        Log.d("GRIB DOWNLOAD", "destinationFileName: " + destinationFileName);
//        fileURL = extras.getString("URL");
        URLList = getIntent().getStringArrayListExtra("URLList");
        fileNameList = getIntent().getStringArrayListExtra("fileNameList");
        nBlock = extras.getInt("niBlock");

        
//        File dFile = new File(rootDir.getAbsolutePath() , fileName);
//        String fileDownloaded = dFile.getAbsolutePath();

//        if(null != fileName)
//            Log.i("GRIB DOWNLOAD", fileDownloaded);


//        Log.i("GRIB DOWNLOAD", fileURL);

        if(!haveNetworkConnection()){
            Intent i = getIntent(); //get the intent that called this activity
            setResult(ERROR_NO_INTERNET, i);
            finish();
        }
        else{
        //executing the asynctask
            Log.i("GRIB DOWNLOAD", " new DFA");
            new DownloadFileAsync().execute(URLList);
        }
    }

    //this is our download file asynctask
    class DownloadFileAsync extends AsyncTask<ArrayList<String>, Integer, String> {

        @Override
        protected void onPreExecute() {
            Log.i("GRIB DOWNLOAD", "onPreExecute");
            
            super.onPreExecute();
            showDialog(DIALOG_DOWNLOAD_PROGRESS);
        }


        @Override
        protected String doInBackground(ArrayList<String>... urls) {

          for (ArrayList<String> urlList : urls) {

            long nAdjustedTotal = 1;
            for(int i=0 ; i < urlList.size() ; i++){

                Log.d("GRIB DOWNLOAD",urlList.get(i));
                Log.d("GRIB DOWNLOAD",fileNameList.get(i));

                int nProgressChunk = 100/urlList.size();
                int progressOffset = nProgressChunk * i;

                HttpURLConnection c;
                InputStream in;
    
                try {
                    //connecting to url
                    URL u = new URL(urlList.get(i));
                    c = (HttpURLConnection) u.openConnection();
                    c.setRequestProperty("Accept-Encoding", "identity");
                    c.setRequestMethod("GET");
                    c.setDoOutput(true);
                    c.connect();
    
                    in = c.getInputStream();
    
                    } catch (Exception e) {
                        Log.d("GRIB DOWNLOAD ExceptionA", e.getMessage());
                        m_result = ERROR_NO_CONNECTION;
                        return null;
                    }
    
                try {
                    int http_status = c.getResponseCode();
                    String statusMsg = String.format("Status: %d\n", http_status);
                    Log.i("GRIB DOWNLOAD", statusMsg);
    
                    //lenghtOfFile is used for calculating download progress
                    int lenghtOfFile = c.getContentLength();
    
                    String msg = String.format("Connection made, file length: %d\n", lenghtOfFile);
                    Log.i("GRIB DOWNLOAD", msg);
    
                    //this is where the file will be seen after the download
                    File nFile = new File(fileNameList.get(i));
                    File nDir = nFile.getParentFile();
                    if(null != nDir)
                        nDir.mkdirs();
                    nFile.createNewFile();
    
                    String fileDownloaded = nFile.getAbsolutePath();
    
                    FileOutputStream f = new FileOutputStream(nFile);
    
    
                        //here’s the download code
                    byte[] buffer = new byte[1024];
                    int len1 = 0;
                    long total = 0;
                    boolean bGotLength = false;
                    long lgt = 0;

                    while ((len1 = in.read(buffer)) > 0) {
                        if(!bGotLength){
                            int firstByte = (0x000000FF & ((int)buffer[12]));
                            int secondByte = (0x000000FF & ((int)buffer[13]));
                            int thirdByte = (0x000000FF & ((int)buffer[14]));
                            int fourthByte = (0x000000FF & ((int)buffer[15]));
                            lgt  = ((long) (firstByte << 24
                                            | secondByte << 16
                                            | thirdByte << 8
                                            | fourthByte))
                                           & 0xFFFFFFFFL;

                            bGotLength = true;
                            lenghtOfFile = (int)lgt;
                         }

                        //  After the first file is loaded, assume all following files are about the same size
                        if(i > 0)
                            lenghtOfFile = (int)nAdjustedTotal;

                        total += len1;

                        int prog = (int)((total * nProgressChunk)/lenghtOfFile);
                        if(prog > nProgressChunk)
                            prog = nProgressChunk;

                        publishProgress(progressOffset + prog);
                        String bmsg = String.format("%d / %d    %d %d\n", total, lenghtOfFile,  progressOffset, prog);
//                        Log.i("GRIB", "Pub: " + bmsg);

                        f.write(buffer, 0, len1);
                    }

                    if(i == 0)
                        nAdjustedTotal = total;

                    f.close();
    
//                    String bmsg = String.format("%d %X %d %d\n", total, total, nBlock, lgt);
//                    Log.i("GRIB", "nBlock Factors: " + bmsg);

                    //  Rename the downloaded file to the passed parameter
    //                Log.d("GRIB DOWNLOAD","Rename: " + fileDownloaded + " to " + fileName);;
                    //File fi = new File(fileDownloaded);
                    //fi.renameTo(fileName);
    
                    m_result = RESULT_OK;
    
                } catch (Exception e) {
                    Log.d("GRIB DOWNLOAD ExceptionB", e.getMessage());
                    m_result = ERROR_EXCEPTION;

                } finally {
                  c.disconnect();
                }

            }  //for  

            //  We now concatenate the GRIB2 files
            Log.d("GRIB DOWNLOAD", "destinationFileName: " + destinationFileName);
            FileOutputStream outputStream = null;
            try{
                outputStream = new FileOutputStream(destinationFileName, true);    // appending
                for(int i=0 ; i < urlList.size() ; i++){
                    InputStream inputStream = new FileInputStream(fileNameList.get(i));
                    Log.d("GRIB DOWNLOAD", "concatenate " + fileNameList.get(i) + " to " + destinationFileName);
                    copyFile(inputStream, outputStream);

                }
            }catch (Exception e) {
                Log.d("GRIB DOWNLOAD Copy Exception", e.getMessage());
                m_result = ERROR_EXCEPTION;
            }finally {
            }




          }   //  vararg



            return "OK";
        }

        protected void onProgressUpdate(Integer... progress) {
             String msg = String.format("%d\n", progress[0]);
//             Log.d("GRIB DOWNLOAD Progress",msg);
             mProgressDialog.setProgress(progress[0]);
        }

        @Override
        protected void onPostExecute(String unused) {
            Log.i("GRIB DOWNLOAD", "onPostExecute");

            //dismiss the dialog after the file was downloaded
            dismissDialog(DIALOG_DOWNLOAD_PROGRESS);

            Bundle b = new Bundle();
//            b.putString("itemSelected", m_selected);
            Intent i = getIntent(); //gets the intent that called this intent
//            i.putExtras(b);
            setResult(m_result, i);

            finish();
        }
    }

    private void copyFile(InputStream inputStream, FileOutputStream outputStream)
        throws IOException
    {
        byte[] buffer = new byte[BUFFER_SIZE];

        int count;
        while ((count = inputStream.read(buffer)) > 0)
            outputStream.write(buffer, 0, count);
    }

    //function to verify if directory exists
    public void checkAndCreateDirectory(String dirName){
        File new_dir = new File( rootDir + dirName );
        if( !new_dir.exists() ){
            new_dir.mkdirs();
        }
    }

    //our progress bar settings
    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIALOG_DOWNLOAD_PROGRESS: //we set this to 0
                mProgressDialog = new ProgressDialog(this);
                mProgressDialog.setMessage("Downloading file…");
                mProgressDialog.setIndeterminate(false);
                mProgressDialog.setMax(100);
                mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                mProgressDialog.setCancelable(true);
                mProgressDialog.show();
                return mProgressDialog;
            default:
                return null;
        }
    }

    private boolean haveNetworkConnection() {
        boolean haveConnectedWifi = false;
        boolean haveConnectedMobile = false;

        ConnectivityManager cm = (ConnectivityManager) getSystemService(this.CONNECTIVITY_SERVICE);
        NetworkInfo[] netInfo = cm.getAllNetworkInfo();
        for (NetworkInfo ni : netInfo) {
            if (ni.getTypeName().equalsIgnoreCase("WIFI"))
                if (ni.isConnected())
                    haveConnectedWifi = true;
            if (ni.getTypeName().equalsIgnoreCase("MOBILE"))
                if (ni.isConnected())
                    haveConnectedMobile = true;
        }
        return haveConnectedWifi || haveConnectedMobile;
    }


}
