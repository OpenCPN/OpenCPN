package org.opencpn.opencpn;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.content.Intent;
//import android.support.design.widget.FloatingActionButton;
//import android.support.design.widget.Snackbar;
//import android.support.v7.app.AlertDialog;
//import android.support.v7.app.AppCompatActivity;
//import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Activity;
import android.net.Uri ;

import org.opencpn.opencpn.R;

import ar.com.daidalos.afiledialog.FileChooserDialog;

public class OCPNChartInstallActivity extends Activity {

    Button selectDirButton;
    Button installButton;
    Button doneButton;

    String m_dirSelected;
    String m_chartzip;
    String m_currentDir = "";
    String m_zipRoot;
    TextView statusText;
    Uri m_chartzipUri;

    boolean m_bComplete = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.content_chart_install);


        Log.i("OpenCPN", "OCPNChartInstallActivity onCreate");

        // Get intent, action and MIME type
        Intent intent = getIntent();
        String action = intent.getAction();
        String type = intent.getType();

        Uri uri = (Uri) intent.getExtras().get(Intent.EXTRA_STREAM);
        m_chartzipUri = uri;
        String sharedFile = uri.toString();

        if (sharedFile != null) {
            Log.i("OpenCPN", "OCPNChartInstallActivity Got uri: " + sharedFile);
            m_chartzip = sharedFile;
        }

        // Extract the passed parameters from the string bundle...
        Bundle extras = intent.getExtras();
        if (extras != null) {
            String installLoc = extras.getString("installLocation");
            if(null != installLoc){
                m_currentDir = installLoc;
            }
         }
         Log.i("OpenCPN", "OCPNChartInstallActivity installLocation: " + m_currentDir);


        statusText = (TextView)findViewById(R.id.statusText);
        statusText.setText(R.string.install_status_ready);


        // Read the chart zip file, and get the name of the root entry
        if( null != m_chartzip ){

            // Name and extension of the shared file
            String fileName = new File(uri.getPath()).getName();
            //  Open the file as an input stream
            InputStream inputStream = null;
            try{
                inputStream = getContentResolver().openInputStream(uri);

                String length = String.valueOf(inputStream.available());
                Log.i("OpenCPN", "OCPNChartInstallActivity: inputStream available(): " + length);
            }
            catch (Exception e) {
                e.printStackTrace();
                Log.i("OpenCPN", "OCPNChartInstallActivity:  Input Stream Exception");
            }

            try{
                ZipInputStream inZip = new ZipInputStream( inputStream );
                ZipEntry root = inZip.getNextEntry();
                m_zipRoot = root.getName();

                TextView view = (TextView) findViewById(R.id.textView6);
                view.setText(m_zipRoot);


            } catch (Exception e) {
                e.printStackTrace();
            }


        }


        addListenerOnButtonSelectDir();
        addListenerOnButtonInstall();
        addListenerOnButtonDone();

        updateGUI();

    }

    public void addListenerOnButtonSelectDir() {

        selectDirButton = (Button) findViewById(R.id.buttonSelectDir);

        final OCPNChartInstallActivity activity = this;
        selectDirButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {

                //String startDir = m_currentDir;
                //if( null == startDir )
                String startDir = activity.getFilesDir().getAbsolutePath();

                final FileChooserDialog dialog = new FileChooserDialog(activity, startDir) ;

                dialog.setShowFullPath( true );
                dialog.setFolderMode( true );
                dialog.setCanCreateFiles( true );


                dialog.setTitle( R.string.install_browser_title );

                dialog.addListener(new FileChooserDialog.OnFileSelectedListener() {
                    public void onFileSelected(Dialog source, File file) {
                        source.hide();
                        //Toast toast = Toast.makeText(source.getContext(), "File selected: " + file.getAbsolutePath(), Toast.LENGTH_LONG);
                        //toast.show();

                        m_dirSelected = file.getAbsolutePath();

                        m_currentDir = m_dirSelected;
                        //TextView view = (TextView) findViewById(R.id.textView8);
                        //view.setText(m_dirSelected);

                        //Log.i("OpenCPN", "Activity onFileSelectedA: " + file.getPath());
                       // m_filechooserString = "file:" + file.getPath();
                        //m_FileChooserDone = true;
                        updateGUI();

                    }
                    public void onFileSelected(Dialog source, File folder, String name) {
                        //Log.i("OpenCPN", "Activity onFileSelectedB: " + folder.getPath() +  File.separator + name);

                        File newFolder = new File(folder.getPath() +  File.separator + name);
                        boolean success = true;
                        if (!newFolder.exists())
                            success = newFolder.mkdirs();

                        if(!success){

                            AlertDialog.Builder builder1 = new AlertDialog.Builder(source.getContext());
                            builder1.setMessage(R.string.install_createdir_error);
                            builder1.setCancelable(true);

                            builder1.setPositiveButton(
                                    "OK",
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(DialogInterface dialog, int id) {
                                            dialog.cancel();
                                        }
                                    });

                            //builder1.setNegativeButton(
                            //        "No",
                            //        new DialogInterface.OnClickListener() {
                            //            public void onClick(DialogInterface dialog, int id) {
                            //                dialog.cancel();
                            //            }
                            //        });

                            AlertDialog alert11 = builder1.create();
                            alert11.show();

                            Toast toast = Toast.makeText(source.getContext(), "Could not create file in: " + folder.getPath(), Toast.LENGTH_LONG);
                            toast.show();
                        }

                        dialog.loadFolder(folder.getPath());
                    }

                });

                //dialog.setOnCancelListener(new OnCancelListener() {
                //    public void onCancel(DialogInterface dialog) {
                //        Log.i("OpenCPN", "DirChooserDialog Cancel");
                //        m_filechooserString = "cancel:";
                //        m_FileChooserDone = true;
                //    }
                //});


                dialog.show();

                //Log.i("DEBUGGER_TAG", "DirChooserDialog Back from show");


            }

        });

    }

    public void addListenerOnButtonInstall() {

        installButton = (Button) findViewById(R.id.buttonInstall);

        installButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {


                if( (m_chartzip.length() > 0) && (m_dirSelected.length() > 0) ){
                    String unzipDir = m_dirSelected;
                    if(!unzipDir.endsWith("/")){
                        unzipDir += "/";
                    }
                    unzip(m_chartzipUri, unzipDir);

                    statusText.setText(R.string.install_complete);
                    m_bComplete = true;
                    updateGUI();

                }

            }

        });
    }

    public void addListenerOnButtonDone() {

            doneButton = (Button) findViewById(R.id.buttonDone);
            doneButton.setVisibility(View.GONE);

            doneButton.setOnClickListener(new View.OnClickListener() {

                @Override
                public void onClick(View arg0) {

                    finish();

                }

            });

    }

    private void updateGUI(){
        Log.i("OpenCPN", "OCPNChartInstallActivity updateGUI m_currentDir: " + m_currentDir);

        if(m_currentDir.length() > 0)
            installButton.setVisibility(View.VISIBLE);
        else
            installButton.setVisibility(View.GONE);

        ((TextView)findViewById(R.id.textView8)).setText(m_currentDir);

        if(m_bComplete){
            TextView tipView = (TextView) findViewById(R.id.tipView);
            tipView.setVisibility(View.INVISIBLE);

            installButton.setVisibility(View.GONE);
            selectDirButton.setVisibility(View.GONE);
            doneButton.setVisibility(View.VISIBLE);
        }
        else{
            doneButton.setVisibility(View.GONE);
        }



    }




    public void unzip(Uri inputUri, String _targetLocation) {
        Log.i("OpenCPN", "OCPNChartInstallActivity ZIP unzipping " + inputUri.toString() + " to " + _targetLocation);

        InputStream inputStream = null;

        try{
            inputStream = getContentResolver().openInputStream(inputUri);

        }
        catch (Exception e) {
            e.printStackTrace();
            Log.i("OpenCPN", "OCPNChartInstallActivity:unzip  Input Stream Exception");
        }


        try {
            ZipInputStream zin = new ZipInputStream(inputStream);
            ZipEntry ze = null;
            while ((ze = zin.getNextEntry()) != null) {

                Log.i("OpenCPN", "ZIP Entry: " + ze.getName());

                //create dir if required while unzipping
                if (ze.isDirectory()) {
                    File dir = new File( _targetLocation + ze.getName());
                    if(!dir.exists()){
                        dir.mkdirs();
                    }

                } else {
                    int size;
                    byte[] buffer = new byte[2048];

                    FileOutputStream outStream = new FileOutputStream(_targetLocation + ze.getName());
                    BufferedOutputStream bufferOut = new BufferedOutputStream(outStream, buffer.length);

                    while((size = zin.read(buffer, 0, buffer.length)) != -1) {
                        bufferOut.write(buffer, 0, size);
                    }

                    bufferOut.flush();
                    bufferOut.close();

                    zin.closeEntry();


                }
            }
            zin.close();
        } catch (Exception e) {
            System.out.println(e);
        }
    }

    @Override
    public void finish() {

        Bundle b = new Bundle();
        b.putString("installLocation", m_currentDir);
        b.putString("rootDir", m_zipRoot);
        Intent intent = new Intent();
        intent.putExtras(b);
        setResult(RESULT_OK, intent);

        super.finish();
    }

}
