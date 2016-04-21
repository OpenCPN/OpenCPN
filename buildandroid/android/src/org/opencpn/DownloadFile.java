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

public class DownloadFile extends Activity {

    //initialize our progress dialog/bar
    private ProgressDialog mProgressDialog;
    public static final int DIALOG_DOWNLOAD_PROGRESS = 0;
    public static final int ERROR_NO_INTERNET = 1;
    public static final int ERROR_NO_CONNECTION = 2;

    public int m_result = RESULT_OK;

    //initialize root directory
    //File rootDir = Environment.getExternalStorageDirectory();
    File rootDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);

    //defining file name and url
    public String fileName = "index.html";
    public String fileURL = "http://192.168.37.99/index.html";

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
        fileName = extras.getString("FILE_NAME");
        fileURL = extras.getString("URL");

        File dFile = new File(rootDir.getAbsolutePath() , fileName);
        String fileDownloaded = dFile.getAbsolutePath();

        if(null != fileName)
            Log.i("GRIB DOWNLOAD", fileDownloaded);


        Log.i("GRIB DOWNLOAD", fileURL);

        if(!haveNetworkConnection()){
            Intent i = getIntent(); //get the intent that called this activity
            setResult(ERROR_NO_INTERNET, i);
            finish();
        }
        else{
        //executing the asynctask
            Log.i("GRIB DOWNLOAD", " new DFA");
            new DownloadFileAsync().execute(fileURL);
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

    //this is our download file asynctask
    class DownloadFileAsync extends AsyncTask<String, String, String> {

        @Override
        protected void onPreExecute() {
            Log.i("GRIB DOWNLOAD", "onPreExecute");
            super.onPreExecute();
            showDialog(DIALOG_DOWNLOAD_PROGRESS);
        }


        @Override
        protected String doInBackground(String... aurl) {

            HttpURLConnection c;
            InputStream in;

            try {
                //connecting to url
                URL u = new URL(fileURL);
                c = (HttpURLConnection) u.openConnection();
                c.setRequestProperty("Accept-Encoding", "identity");
                c.setRequestMethod("GET");
                c.setDoOutput(true);
                c.connect();

                in = c.getInputStream();

                } catch (Exception e) {
                    Log.d("GRIB DOWNLOAD Exception", e.getMessage());
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
                File nFile = new File(rootDir.getAbsolutePath() , fileName);
                nFile.createNewFile();

                String fileDownloaded = nFile.getAbsolutePath();

                FileOutputStream f = new FileOutputStream(nFile);


                    //here’s the download code
                byte[] buffer = new byte[1024];
                int len1 = 0;
                long total = 0;

                while ((len1 = in.read(buffer)) > 0) {
                    total += len1; //total = total + len1
                    publishProgress("" + (int)((total*100)/lenghtOfFile));
                    f.write(buffer, 0, len1);
                }
                f.close();

                //  Rename the downloaded file to the passed parameter
//                Log.d("GRIB DOWNLOAD","Rename: " + fileDownloaded + " to " + fileName);;
                //File fi = new File(fileDownloaded);
                //fi.renameTo(fileName);

                m_result = RESULT_OK;

            } catch (Exception e) {
                Log.d("GRIB DOWNLOAD Exception", e.getMessage());
            } finally {
              c.disconnect();
            }


            return null;
        }

        protected void onProgressUpdate(String... progress) {
             Log.d("GRIB DOWNLOAD",progress[0]);
             mProgressDialog.setProgress(Integer.parseInt(progress[0]));
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
}
