/*
 * <Copyright 2013 Jose F. Maldonado>
 *
 *  This file is part of aFileDialog.
 *
 *  aFileDialog is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published 
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  aFileDialog is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with aFileDialog. If not, see <http://www.gnu.org/licenses/>.
 */

package ar.com.daidalos.afiledialog;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;

import android.support.v4.provider.DocumentFile;

import android.view.ContextThemeWrapper;
import android.view.View;
import android.widget.LinearLayout;
import ar.com.daidalos.afiledialog.R;
import android.widget.TextView;

/**
 * A file chooser implemented in an Activity. 
 */
public class FileChooserActivity extends Activity implements FileChooser {
	
	// ----- Fields ----- //

    public String m_saf43CreateDir;
	/**
	 * The folder that the class opened by default. 
	 */
	private File startFolder;
	
	/**
	 * The core of the file chooser.
	 */
	private FileChooserCore core;
	
	/**
	 * A boolean indicating if the 'back' button must be used to navigate to parent folders.
	 */
	private boolean useBackButton;
	
	// ----- Constants ----- //
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains the 
	 * path of the folder which files are going to be listed. 
	 */
	public static final String INPUT_START_FOLDER = "input_start_folder";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * a boolean that indicates if the user is going to select folders instead of select files. 
	 */
	public static final String INPUT_FOLDER_MODE = "input_folder_mode";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * a boolean that indicates if the user can create files. 
	 */
	public static final String INPUT_CAN_CREATE_FILES = "input_can_create_files";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains 
	 * a regular expression which is going to be used as a filter to determine which files can be selected. 
	 */
	public static final String INPUT_REGEX_FILTER = "input_regex_filter";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * a boolean that indicates if only the files that can be selected must be displayed.
	 */
	public static final String INPUT_SHOW_ONLY_SELECTABLE = "input_show_only_selectable";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * an instance of the class FileChooserLabels that allows to override the default value of the labels.
	 */
	public static final String INPUT_LABELS = "input_labels";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * a boolean that indicates if a confirmation dialog must be displayed when creating a file.
	 */
	public static final String INPUT_SHOW_CONFIRMATION_ON_CREATE = "input_show_confirmation_on_create";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * a boolean that indicates if a confirmation dialog must be displayed when selecting a file.
	 */
	public static final String INPUT_SHOW_CONFIRMATION_ON_SELECT = "input_show_confirmation_on_select";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * a boolean that indicates if the title must show the full path of the current's folder (true) or only
	 * the folder's name (false).
	 */
	public static final String INPUT_SHOW_FULL_PATH_IN_TITLE = "input_show_full_path_in_title";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the start's intent) which contains  
	 * a boolean that indicates if the 'Back' button must be used to navigate to the parents folder (true) or
	 * if must follow the default behavior (and close the activity when the button is pressed).
	 */
	public static final String INPUT_USE_BACK_BUTTON_TO_NAVIGATE = "input_use_back_button_to_navigate";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the result's intent) which contains the 
	 * File object, that represents the file selected by the user or the folder in which the user wants to create
	 * a file. 
	 */
	public static final String OUTPUT_FILE_OBJECT = "output_file_object";
	
	/**
	 * Constant used for represent the key of the bundle object (inside the result's intent) which contains the 
	 * name of the file that the user wants to create.
	 */
	public static final String OUTPUT_NEW_FILE_NAME = "output_new_file_name";
	
        public static final String INPUT_TITLE_STRING = "Title Message";

        public static final String INPUT_SORT_LAST_MODIFIED = "sort_by_last_modified";

	// ---- Activity methods ----- //
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
		// Call superclass creator.
        super.onCreate(savedInstanceState);
        
		// Set layout.
		this.setContentView(R.layout.daidalos_file_chooser);
		
		// Set the background color.
        LinearLayout layout = (LinearLayout) this.findViewById(R.id.rootLayout);
        layout.setBackgroundColor(getResources().getColor(R.color.daidalos_backgroud));

		// Initialize fields.
		this.useBackButton = false;
		
        // Create the core of the file chooser.
        this.core = new FileChooserCore(this);
        
        // Verify if the optional parameters has been defined.
        String folderPath = null;
        Bundle extras = this.getIntent().getExtras();
        if(extras != null) {
    		if(extras.containsKey(INPUT_START_FOLDER)) folderPath = extras.getString(INPUT_START_FOLDER);
            if(extras.containsKey(INPUT_TITLE_STRING)) this.setTitle(extras.getString(INPUT_TITLE_STRING));
            if(extras.containsKey(INPUT_REGEX_FILTER)) core.setFilter(extras.getString(INPUT_REGEX_FILTER));
            if(extras.containsKey(INPUT_SHOW_ONLY_SELECTABLE)) core.setShowOnlySelectable(extras.getBoolean(INPUT_SHOW_ONLY_SELECTABLE));
            if(extras.containsKey(INPUT_FOLDER_MODE)) core.setFolderMode(extras.getBoolean(INPUT_FOLDER_MODE));
            if(extras.containsKey(INPUT_CAN_CREATE_FILES)) core.setCanCreateFiles(extras.getBoolean(INPUT_CAN_CREATE_FILES));
            if(extras.containsKey(INPUT_LABELS)) core.setLabels((FileChooserLabels) extras.get(INPUT_LABELS));
            if(extras.containsKey(INPUT_SHOW_CONFIRMATION_ON_CREATE)) core.setShowConfirmationOnCreate(extras.getBoolean(INPUT_SHOW_CONFIRMATION_ON_CREATE));
            if(extras.containsKey(INPUT_SHOW_CONFIRMATION_ON_SELECT)) core.setShowConfirmationOnSelect(extras.getBoolean(INPUT_SHOW_CONFIRMATION_ON_SELECT));
            if(extras.containsKey(INPUT_SHOW_FULL_PATH_IN_TITLE)) core.setShowFullPathInTitle(extras.getBoolean(INPUT_SHOW_FULL_PATH_IN_TITLE));
            if(extras.containsKey(INPUT_USE_BACK_BUTTON_TO_NAVIGATE)) this.useBackButton = extras.getBoolean(INPUT_USE_BACK_BUTTON_TO_NAVIGATE);
            if(extras.containsKey(INPUT_SORT_LAST_MODIFIED)) core.setSortLastModified(extras.getBoolean(INPUT_SORT_LAST_MODIFIED));
        }

        // Load the files of a folder.
        core.loadFolder(folderPath);
        this.startFolder = this.core.getCurrentFolder();
        
        // Add a listener for when a folder is created.
        core.addListener(new FileChooserCore.OnFileSelectedListener() {

                public void onFileSelected(File folder, String name) {
                //Log.i("OpenCPN", "FileChooserActivity::OnFileSelected(folder,name)");

                final File newFolder = new File(folder.getPath() +  File.separator + name);

                boolean success = true;
                if (!newFolder.exists())
                    success = newFolder.mkdirs();

                if(!success) {

                    //  Is this on an SDCard?
                    String sdRoot = getExtSdCardFolder(folder);
                    if (null != sdRoot) {


                        // This will create the dir, if possible
                        DocumentFile f = getDocumentFile(newFolder, true, true);
                        if (null == f) {
                            m_saf43CreateDir = newFolder.getAbsolutePath();
                            startSAFDialog(43);
                            return;
                        }
                        else{

                            File current = core.getCurrentFolder();
                            core.loadFolder(current);

                             return;
                        }
                    } else {
                        //showCreateErrorDialog(folder);
                    }
                }













                // Pass the data through an intent.
				Intent intent = new Intent();
				Bundle bundle = new Bundle();
				bundle.putSerializable(OUTPUT_FILE_OBJECT, folder);
				bundle.putString(OUTPUT_NEW_FILE_NAME, name);
				intent.putExtras(bundle);
				
                setResult(RESULT_OK, intent);
                finish();				
			}

                public void onFileSelected(File file) {

                //Log.i("OpenCPN", "FileChooserActivity::OnFileSelected(file)");


                //String m_dirSelected = file.getAbsolutePath();

                ;

                //  Is this on an SDCard?
                if (android.os.Build.VERSION.SDK_INT > android.os.Build.VERSION_CODES.KITKAT) {
                    String sdRoot = getExtSdCardFolder(file);
                    if (null != sdRoot) {

                        // Accessible?
                        DocumentFile dir = getDocumentFile(file, true, false);
                        if (null == dir) {
                            startSAFDialog(44);
                            return;
                        }

                        if (!dir.canWrite()) {
                            startSAFDialog(44);
                            return;
                        }
                    }
                }

                // Pass the data through an intent.
				Intent intent = new Intent();
				Bundle bundle = new Bundle();
				bundle.putSerializable(OUTPUT_FILE_OBJECT, file);
				intent.putExtras(bundle);
				
                setResult(RESULT_OK, intent);
                finish();	
			}
		});
    }
    
    /** Called when the user push the 'back' button. */
    @Override
    public void onBackPressed() {
    	// Verify if the activity must be finished or if the parent folder must be opened.
   	    File current = this.core.getCurrentFolder();
   	    if(!this.useBackButton || current == null || current.getParent() == null || current.getPath().compareTo(this.startFolder.getPath()) == 0) {
   	    	// Close activity.
   	    	super.onBackPressed();
   	    }else{
   	    	// Open parent.
   	        this.core.loadFolder(current.getParent());
   	    }
  	}

    private void startSAFDialog(final int code){
        ContextThemeWrapper ctw = new ContextThemeWrapper(this, R.style.AlertTheme1);
        AlertDialog.Builder builder1 = new AlertDialog.Builder(ctw);

        String msg = getString(R.string.sdcard_permission_help);
        builder1.setMessage(msg);
        builder1.setCancelable(true);

        builder1.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();

                        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
                        intent.addFlags(Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
                        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                        startActivityForResult(intent, code);


                    }
                });


        AlertDialog alert11 = builder1.create();
        alert11.show();
    }

    /**
     * Get a list of external SD card paths. (Kitkat or higher.)
     *
     * @return A list of external SD card paths.
     */
    private String[] getExtSdCardPaths() {
        List<String> paths = new ArrayList();



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



// ----- FileChooser methods ----- //
    
	public LinearLayout getRootLayout() {
		View root = this.findViewById(R.id.rootLayout); 
		return (root instanceof LinearLayout)? (LinearLayout)root : null;
	}

	public Context getContext() {
		//return this.getBaseContext();
		return this;
	}
	
	public void setCurrentFolderName(String name) {
                //this.setTitle(name);

                TextView selected = (TextView)this.findViewById(R.id.afdLabelFile);
                if(null != selected){
                    selected.setText( name );
                }


	}

	@Override
	public final void onActivityResult(final int requestCode, final int resultCode, final Intent resultData) {
		if (requestCode == 43) {
			Uri treeUri = null;
			if (resultCode == Activity.RESULT_OK) {
				// Get Uri from Storage Access Framework.
				treeUri = resultData.getData();

				String a = treeUri.toString();

				// Persist URI in shared preference so that you can use it later.
                SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
                SharedPreferences.Editor editor = preferences.edit();
                editor.putString("SDURI", treeUri.toString());
                editor.commit();


                // Create the dir
                if(!m_saf43CreateDir.isEmpty()) {
                    File newDir = new File(m_saf43CreateDir);
                    DocumentFile dir = getDocumentFile(newDir, true, true);
                }

				// Persist access permissions.
//		final int takeFlags = resultData.getFlags();
                final int takeFlags = resultData.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

//                getContentResolver().takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                getContentResolver().takePersistableUriPermission(treeUri, takeFlags);

                File current = this.core.getCurrentFolder();
                this.core.loadFolder(current);

                showPermisionGrantedDialog();

			}
		}else if (requestCode == 44) {
            Uri treeUri = null;

            if (resultCode == Activity.RESULT_OK) {

                treeUri = resultData.getData();


                // Persist URI in shared preference so that you can use it later.
                SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
                SharedPreferences.Editor editor = preferences.edit();
                editor.putString("SDURI", treeUri.toString());
                editor.commit();


                getContentResolver().takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

                File current = this.core.getCurrentFolder();
                this.core.loadFolder(current);

                showPermisionGrantedDialog();

            }
        }

    }

    private void showPermisionGrantedDialog() {
        ContextThemeWrapper ctw = new ContextThemeWrapper(this, R.style.AlertTheme1);
        AlertDialog.Builder builder1 = new AlertDialog.Builder(ctw);

        builder1.setMessage(R.string.permission_granted);
        builder1.setCancelable(true);

        builder1.setPositiveButton(R.string.daidalos_ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });


        AlertDialog alert11 = builder1.create();
        alert11.show();

    }


}
