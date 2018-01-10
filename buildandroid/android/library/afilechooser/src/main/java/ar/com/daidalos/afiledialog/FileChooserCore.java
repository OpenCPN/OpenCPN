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

import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;

import android.app.AlertDialog;
import android.app.Application;
import android.content.Context;
import android.content.DialogInterface;
import android.content.UriPermission;
import android.os.Environment;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import ar.com.daidalos.afiledialog.view.FileItem;
import android.app.Dialog;
import android.widget.ProgressBar;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ProgressBar;
import android.os.AsyncTask;
import android.app.ProgressDialog;
import android.util.Log;

import android.content.Context;
import java.util.HashMap;
import java.util.Map;
import android.os.Environment;
import android.annotation.TargetApi;
import android.content.Intent;

/**
 * This class implements the common features of a file chooser.
 */
class FileChooserCore {

	// ----- Attributes ----- //

	/**
	 * The file chooser in which all the operations are performed.
	 */
	private FileChooser chooser;

	/**
	 * The listeners for the event of select a file.
	 */
	private List<OnFileSelectedListener> listeners;

	/**
	 * A regular expression for filter the files.
	 */
	private String filter;

	/**
	 * A boolean indicating if only the files that can be selected (they pass the filter) must be show.
	 */
	private boolean showOnlySelectable;

	/**
	 * A boolean indicating if the user can create files.
	 */
	private boolean canCreateFiles;

	/**
	 * A boolean indicating if the chooser is going to be used to select folders.
	 */
	private boolean folderMode;

	/**
	 * A file that indicates the folder that is currently being displayed.
	 */
	private File currentFolder;

	/**
	 * This attribut allows to override the default value of the labels.
	 */
	private FileChooserLabels labels;

	/**
	 * A boolean that indicates if a confirmation dialog must be displaying when selecting a file.
	 */
	private boolean showConfirmationOnSelect;

	/**
	 * A boolean that indicates if a confirmation dialog must be displaying when creating a file.
	 */
	private boolean showConfirmationOnCreate;

	/**
	 * A boolean indicating if the folder's full path must be show in the title.
	 */
        private boolean showFullPathInTitle = true;


        private boolean sortLastModified;

	// ---- Static attributes ----- //

	/**
	 * Static attribute for save the folder displayed by default.
	 */
	private static File defaultFolder;

	/**
	 * Static constructor.
	 */
	static {
		defaultFolder = null;
	}

        ProgressBar m_progressBar;

        String m_sdcardDir = null;

        public void setDone( boolean d){
                        m_done = d;
                }

        public void setLinkedList( LinkedList<FileItem> ll){
                        m_linkedList = ll;
                }

        private boolean m_done;
        private LinkedList<FileItem> m_linkedList;

        public GetFilesTask m_task;

                private class GetFilesTask extends AsyncTask<FileChooserCore, Integer, Void> {

                        private ProgressDialog dialog;
                        private FileChooserCore m_core;

                        public GetFilesTask(FileChooserCore core) {
                                dialog = new ProgressDialog(chooser.getContext(), 3 );
                        //	dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                        //	dialog.setProgressStyle(R.style.NewDialogX);
                                dialog.setIndeterminate(true);
                                dialog.setTitle("Scanning Files");
                                dialog.setMessage("Please wait...");
                                m_core = core;
                        }


                        @Override
                        protected void onPreExecute() {


                        //	;
                        }


                        protected Void doInBackground(FileChooserCore... core) {

                                LinkedList<FileItem> fileItems = new LinkedList<FileItem>();


                                File[] fileList = currentFolder.listFiles();

                                if(fileList != null){

                                        // Iterate all the files in the folder.
                                        for(int i=0; i<fileList.length; i++) {
                                                publishProgress(i);
                                                // Verify if file can be selected (is a directory or folder mode is not activated and the file pass the filter, if defined).
                                                boolean selectable = true;
                                                if(!fileList[i].isDirectory()) {
                                                        selectable = !folderMode && (filter == null || fileList[i].getName().matches(filter));
                                                }

                                                // Verify if the file must be show.
                                                if (selectable || !showOnlySelectable) {
                                                        // Create the file item and add it to the list.
                                                        FileItem fileItem = new FileItem(chooser.getContext(), fileList[i]);
                                                        fileItem.setSelectable(selectable);
                                                        fileItems.add(fileItem);
                                                }
                                        }

                                }
                                //setList(fileList);
                                setLinkedList(fileItems);
                                return  null;
                        }

                        @Override
                        protected void onProgressUpdate(Integer... progress) {
                                dialog.show();
                        }

                        @Override
                        protected void onPostExecute(Void result) {
                                this.dialog.hide();
                                m_core.CompleteGetFiles();

                        }
                }

                public void CompleteGetFiles(){
                        // Add click listener and add the FileItem objects to the layout.
                        LinearLayout root = this.chooser.getRootLayout();
                        LinearLayout layout = (LinearLayout) root.findViewById(R.id.linearLayoutFiles);

                        // Add the parent folder.
                        if(this.currentFolder.getParent() != null) {
                                File parent = new File(this.currentFolder.getParent());
                                if(parent.exists()) {
                                        FileItem par = new FileItem(this.chooser.getContext(), parent, "..");
                                        par.addListener(this.fileItemClickListener);
                                        layout.addView( par );
                                }

                        }

                        for(int i=0; i<m_linkedList.size(); i++) {
                                m_linkedList.get(i).addListener(this.fileItemClickListener);
                                layout.addView(m_linkedList.get(i));
                        }

                        // Refresh default folder.
                        defaultFolder = this.currentFolder;

                        setDone(true);

                }

	// ----- Constructor ----- //

	/**
	 * Creates an instance of this class.
	 *
	 * @param fileChooser The graphical file chooser.
	 */
	public FileChooserCore(FileChooser fileChooser) {

                m_sdcardDir = isRemovableSDCardAvailable();
                Log.e("OpenCPN", "FileChooserCore::m_sdcardDir: " + m_sdcardDir);

		// Initialize attributes.
		this.chooser = fileChooser;
		this.listeners = new LinkedList<OnFileSelectedListener>();
		this.filter = null;
		this.showOnlySelectable = false;
		this.setCanCreateFiles(false);
		this.setFolderMode(false);
		this.currentFolder = null;
		this.labels = null;
		this.showConfirmationOnCreate = false;
		this.showConfirmationOnSelect = false;
                this.showFullPathInTitle = true;
                this.sortLastModified = false;

                LinearLayout root = this.chooser.getRootLayout();
                m_progressBar = (ProgressBar) root.findViewById(R.id.progressBar);

		// Add listener for the  buttons.
		Button addButton = (Button) root.findViewById(R.id.buttonAdd);
		addButton.setOnClickListener(addButtonClickListener);
		Button okButton = (Button) root.findViewById(R.id.buttonOk);
		okButton.setOnClickListener(okButtonClickListener);

                Button deviceButton = (Button) root.findViewById(R.id.buttonDevice);
                deviceButton.setOnClickListener(deviceButtonClickListener);
                Button sdcardButton = (Button) root.findViewById(R.id.buttonSDCard);
                sdcardButton.setOnClickListener(sdcardButtonClickListener);

                if(null == m_sdcardDir){
                    sdcardButton.setVisibility( View.INVISIBLE);
                }
                else{
                 sdcardButton.setEnabled(true);
                 sdcardButton.setVisibility( View.VISIBLE);
                }


	}

        /**
         * Check if can create file on given directory. Use this enclose with method
         * {@link BeginScreenFragement#isRemovableSDCardAvailable()} to check sd
         * card is available on device or not.
         *
         * @param directory
         * @return
         */
        public String canCreateFile(String directory) {
            final String FILE_DIR = directory + File.separator + "hoang.txt";
            File tempFlie = null;
            try {
                tempFlie = new File(FILE_DIR);
                FileOutputStream fos = new FileOutputStream(tempFlie);
                fos.write(new byte[1024]);
                fos.flush();
                fos.close();
                Log.e("OpenCPN", "Can write file on this directory: " + FILE_DIR);
            } catch (Exception e) {
                Log.e("OpenCPN", "Write file error: " + e.getMessage());
                return null;
            } finally {
                if (tempFlie != null && tempFlie.exists() && tempFlie.isFile()) {
                    tempFlie.delete();
                    tempFlie = null;
                }
            }
            return directory;
        }

        public String isRemovableSDCardAvailable() {
                String directory = null;

                File externalStorageList[] = null;
                int version = android.os.Build.VERSION.SDK_INT;

                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {        // KITKAT = 19/20
                        Log.e("OpenCPN", "Found SDK 19 or greater");
                        externalStorageList = this.chooser.getContext().getExternalFilesDirs(null);

                        if(null != externalStorageList){
                                Log.e("OpenCPN", "eSL not null");


                                for(int i = 0 ; i < externalStorageList.length ; i++){
                                        Log.e("OpenCPN", String.valueOf(i) + "/ " + String.valueOf(externalStorageList.length));
                                        if(null != externalStorageList[i])
                                                Log.e("OpenCPN", "eSL content: " + String.valueOf(i) + " " + externalStorageList[i].getAbsolutePath());
                                }

                                if(android.os.Build.VERSION.SDK_INT >= 21){
                                        // Lollipop snd later takes this path
                                        Log.e("OpenCPN", "Found SDK 21 or greater");
                                        for(int i = 0 ; i < externalStorageList.length ; i++){
                                                if(null != externalStorageList[i]){
                                                        directory = externalStorageList[i].getAbsolutePath();

                                                        Log.e("OpenCPN", "Checking directory: " + directory);

                                                        if(android.os.Environment.isExternalStorageRemovable(externalStorageList[i])) {  // added in API 21
                                                                Log.e("OpenCPN", "... is removable");

                                                                File dirTest = new File(directory);
                                                                if(!dirTest.exists()){
                                                                    Log.e("OpenCPN", "Creating: " + directory);
                                                                    try{
                                                                        dirTest.createNewFile();
                                                                    } catch(IOException e){
                                                                       Log.e("OpenCPN", "Exception creating: " + directory);
                                                                       directory = null;
                                                                    }

                                                                }

                                                                if(canCreateFiles){
                                                                    directory = canCreateFile(directory);
                                                                    if (null != directory) {
                                                                            Log.e("OpenCPN", "... is writable");
                                                                            Log.e("OpenCPN", "SD Card's directory: " + directory);
                                                                            return directory;
                                                                    }
                                                                }
                                                                else{
                                                                    Log.e("OpenCPN", "... only need R/O");
                                                                    Log.e("OpenCPN", "SD Card's directory: " + directory);
                                                                    return directory;
                                                                }


                                                        }
                                                }
                                        }
                                }
                                else {
                                        // KitKat takes this path
                                        Log.e("OpenCPN", "Found SDK 19/20");
                                        if(externalStorageList.length > 1){
                                            if(null != externalStorageList[1]){
                                                // Force to use the second in the list
                                            directory = externalStorageList[1].getAbsolutePath();
                                            Log.e("OpenCPN", "Checking directory: " + directory);

                                            return directory;
/*
                                                directory = canCreateFile(directory);
                                                if (null != directory) {
                                                        Log.e("OpenCPN", "... is writable");
                                                        Log.e("OpenCPN", "SD Card's directory: " + directory);
                                                        return directory;
                                                }

*/

                                            }

                                        }
                                }
                        }
                }


                // Pre 19 (KitKat), or other problem...
                Log.e("OpenCPN", "Found SDK less than 19");

                final String FLAG = "mnt";

                String SECONDARY_STORAGE = System.getenv("SECONDARY_STORAGE");
                // Split this line, and take only the first element
                final String[] items = SECONDARY_STORAGE.split(":");
                SECONDARY_STORAGE = items[0];

                final String EXTERNAL_STORAGE_DOCOMO = System.getenv("EXTERNAL_STORAGE_DOCOMO");
                final String EXTERNAL_SDCARD_STORAGE = System.getenv("EXTERNAL_SDCARD_STORAGE");
                final String EXTERNAL_SD_STORAGE = System.getenv("EXTERNAL_SD_STORAGE");
                final String EXTERNAL_STORAGE = System.getenv("EXTERNAL_STORAGE");

                Map<Integer, String> listEnvironmentVariableStoreSDCardRootDirectory = new HashMap<Integer, String>();
                listEnvironmentVariableStoreSDCardRootDirectory.put(0, SECONDARY_STORAGE);
                listEnvironmentVariableStoreSDCardRootDirectory.put(1, EXTERNAL_STORAGE_DOCOMO);
                listEnvironmentVariableStoreSDCardRootDirectory.put(2, EXTERNAL_SDCARD_STORAGE);
                listEnvironmentVariableStoreSDCardRootDirectory.put(3, EXTERNAL_SD_STORAGE);
                listEnvironmentVariableStoreSDCardRootDirectory.put(4, EXTERNAL_STORAGE);


                for(int i = 0 ; i < listEnvironmentVariableStoreSDCardRootDirectory.size() ; i++){
                        Log.e("OpenCPN", "Environment list content: " + String.valueOf(i) + " " + listEnvironmentVariableStoreSDCardRootDirectory.get(i));
                }

                directory = null;
                int size = listEnvironmentVariableStoreSDCardRootDirectory.size();
                for (int i = 0; i < size; i++) {
                        directory = listEnvironmentVariableStoreSDCardRootDirectory.get(i);

                        Log.e("OpenCPN", "Checking directory: " + directory);

                        if(directory != null)
                                directory = canCreateFile(directory);

                        if (directory != null && directory.length() != 0) {
                                if (i == size - 1) {
                                        if (directory.contains(FLAG)) {
                                                Log.e("OpenCPN", "SD Card's directory w/FLAG: " + directory);
                                                return directory;
                                        } else {
                                                Log.e("OpenCPN", "SD Card's directory wo/FLAG: " + directory);
                                                Log.e("OpenCPN", "Writable SD Card directory not found. ");
                                                return null;
                                        }
                                }
                                Log.e("OpenCPN", "SD Card's directory: " + directory);
                                return directory;
                        }
                }

                Log.e("OpenCPN", "Writable SD Card directory not found. ");
                return null;

        }
/*
        private void triggerStorageAccessFramework() {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
            startActivityForResult(intent, REQUEST_CODE_STORAGE_ACCESS);
        }

        @Override
        public final void onActivityResult(final int requestCode, final int resultCode, final Intent resultData) {
            if (requestCode == SettingsFragment.REQUEST_CODE_STORAGE_ACCESS) {
                Uri treeUri = null;
                if (resultCode == Activity.RESULT_OK) {
                    // Get Uri from Storage Access Framework.
                    treeUri = resultData.getData();

                    // Persist URI in shared preference so that you can use it later.
                    // Use your own framework here instead of PreferenceUtil.
                    //PreferenceUtil.setSharedPreferenceUri(R.string.key_internal_uri_extsdcard, treeUri);

                    // Persist access permissions.
                    final int takeFlags = resultData.getFlags()
                        & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                    getActivity().getContentResolver().takePersistableUriPermission(treeUri, takeFlags);
                }
            }
        }

*/

	// ----- Events methods ----- //

	/**
	 * Implementation of the click listener for when the add button is clicked.
	 */
	private View.OnClickListener addButtonClickListener = new View.OnClickListener() {
		public void onClick(View v) {
			// Get the current context.
			Context context = v.getContext();

			// Create an alert dialog.
			AlertDialog.Builder alert = new AlertDialog.Builder(context);

			// Define the dialog's labels.
			String title = context.getString(FileChooserCore.this.folderMode? R.string.daidalos_create_folder : R.string.daidalos_create_file);
			if(FileChooserCore.this.labels != null && FileChooserCore.this.labels.createFileDialogTitle != null) title = FileChooserCore.this.labels.createFileDialogTitle;
			String message = context.getString(FileChooserCore.this.folderMode? R.string.daidalos_enter_folder_name : R.string.daidalos_enter_file_name);
			if(FileChooserCore.this.labels != null && FileChooserCore.this.labels.createFileDialogMessage != null) message = FileChooserCore.this.labels.createFileDialogMessage;
			String posButton = (FileChooserCore.this.labels != null && FileChooserCore.this.labels.createFileDialogAcceptButton != null)? FileChooserCore.this.labels.createFileDialogAcceptButton : context.getString(R.string.daidalos_accept);
			String negButton = (FileChooserCore.this.labels != null && FileChooserCore.this.labels.createFileDialogCancelButton != null)? FileChooserCore.this.labels.createFileDialogCancelButton : context.getString(R.string.daidalos_cancel);

			// Set the title and the message.
			alert.setTitle( title );
			alert.setMessage( message );

			// Set an EditText view to get the file's name.
			final EditText input = new EditText(context);
			input.setSingleLine();
			alert.setView(input);

			// Set the 'ok' and 'cancel' buttons.
			alert.setPositiveButton(posButton, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					String fileName = input.getText().toString();
					// Verify if a value has been entered.
					if(fileName != null && fileName.length() > 0) {
						// Notify the listeners.
						FileChooserCore.this.notifyListeners(FileChooserCore.this.currentFolder, fileName);
					}
				}
			});
			alert.setNegativeButton(negButton, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					// Do nothing, automatically the dialog is going to be closed.
				}
			});

			// Show the dialog.
			alert.show();
		}
	};

	/**
	 * Implementation of the click listener for when the ok button is clicked.
	 */
	private View.OnClickListener okButtonClickListener = new View.OnClickListener() {
		public void onClick(View v) {
			// Notify the listeners.
			FileChooserCore.this.notifyListeners(FileChooserCore.this.currentFolder, null);
		}
	};

        /**
         * Implementation of the click listener for when the "device" button is clicked.
         */
        private View.OnClickListener deviceButtonClickListener = new View.OnClickListener() {
                public void onClick(View v) {
                    loadFolder(Environment.getExternalStorageDirectory());
                }
        };

        /**
         * Implementation of the click listener for when the "SD card" button is clicked.
         */
        private View.OnClickListener sdcardButtonClickListener = new View.OnClickListener() {
                public void onClick(View v) {
                    if(null != m_sdcardDir) {
                        File sdFile = new File(m_sdcardDir);
                        String sdRoot = getExtSdCardFolder(sdFile);
                        if(null != sdRoot)
                            loadFolder(sdRoot);
                    }
                }
        };

        /**
	 * Implementation of the click listener for when a file item is clicked.
	 */
	private FileItem.OnFileClickListener fileItemClickListener = new FileItem.OnFileClickListener() {
		public void onClick(FileItem source) {
			// Verify if the item is a folder.
			File file = source.getFile();
                        //Log.i("OpenCPN", "OnFileClickListener " + file);
            if(file.isDirectory()) {
				// Open the folder.
                                String[] Files = file.list();
                                if(Files != null){
                                    if(Files.length > 10)
                                        FileChooserCore.this.loadFolderA(file);
                                    else
                                        FileChooserCore.this.loadFolder(file);
                                }
                                else{
                                    // Is this an SDCard?
                                    String baseFolder = getExtSdCardFolder(file);
                                    if(null != baseFolder){
                                        FileChooserCore.this.currentFolder = new File(baseFolder);    //Environment.getExternalStorageDirectory();
                                        // Reload the list of files.
                                        FileChooserCore.this.loadFolder(FileChooserCore.this.currentFolder);

                                    }
                                    else {
                                        //  Probably a prohibited read.  Switch to a known good directory.
                                        FileChooserCore.this.currentFolder = Environment.getExternalStorageDirectory();
                                        // Reload the list of files.
                                        FileChooserCore.this.loadFolder(FileChooserCore.this.currentFolder);
                                    }

                                }

                updateSourceGUI(file);

			} else {
				// Notify the listeners.
				FileChooserCore.this.notifyListeners(file, null);
			}
		}
	};

	/**
	 * Add a listener for the event of a file selected.
	 *
	 * @param listener The listener to add.
	 */
	public void addListener(OnFileSelectedListener listener) {
		this.listeners.add(listener);
	}

	/**
	 * Removes a listener for the event of a file selected.
	 *
	 * @param listener The listener to remove.
	 */
	public void removeListener(OnFileSelectedListener listener) {
		this.listeners.remove(listener);
	}

	/**
	 * Removes all the listeners for the event of a file selected.
	 */
	public void removeAllListeners() {
		this.listeners.clear();
	}

	/**
	 * Interface definition for a callback to be invoked when a file is selected.
	 */
	public interface OnFileSelectedListener {
		/**
		 * Called when a file has been selected.
		 *
		 * @param file The file selected.
		 */
		void onFileSelected(File file);

		/**
		 * Called when an user wants to be create a file.
		 *
		 * @param folder The file's parent folder.
		 * @param name The file's name.
		 */
		void onFileSelected(File folder, String name);
	}

	/**
	 * Notify to all listeners that a file has been selected or created.
	 *
	 * @param file The file or folder selected or the folder in which the file must be created.
	 * @param name The name of the file that must be created or 'null' if a file was selected (instead of being created).
	 */
	private void notifyListeners(final File file, final String name) {
		// Determine if a file has been selected or created.
		final boolean creation = name != null && name.length() > 0;

		// Verify if a confirmation dialog must be show.
		if((creation && this.showConfirmationOnCreate || !creation && this.showConfirmationOnSelect)) {
			// Create an alert dialog.
			Context context = this.chooser.getContext();
			AlertDialog.Builder alert = new AlertDialog.Builder(context);

			// Define the dialog's labels.
			String message = null;
			if(FileChooserCore.this.labels != null && ((creation && FileChooserCore.this.labels.messageConfirmCreation != null) || (!creation && FileChooserCore.this.labels.messageConfirmSelection != null)))  {
				message = creation? FileChooserCore.this.labels.messageConfirmCreation : FileChooserCore.this.labels.messageConfirmSelection;
			} else {
				if(FileChooserCore.this.folderMode) {
					message = context.getString(creation? R.string.daidalos_confirm_create_folder : R.string.daidalos_confirm_select_folder);
				} else {
					message = context.getString(creation? R.string.daidalos_confirm_create_file : R.string.daidalos_confirm_select_file);
				}
			}
			if(message != null) message = message.replace("$file_name", name!=null? name : file.getName());
			String posButton = (FileChooserCore.this.labels != null && FileChooserCore.this.labels.labelConfirmYesButton != null)? FileChooserCore.this.labels.labelConfirmYesButton : context.getString(R.string.daidalos_yes);
			String negButton = (FileChooserCore.this.labels != null && FileChooserCore.this.labels.labelConfirmNoButton != null)? FileChooserCore.this.labels.labelConfirmNoButton : context.getString(R.string.daidalos_no);

			// Set the message and the 'yes' and 'no' buttons.
			alert.setMessage( message );
			alert.setPositiveButton(posButton, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					// Notify to listeners.
					for(int i=0; i<FileChooserCore.this.listeners.size(); i++) {
						if(creation) {
							FileChooserCore.this.listeners.get(i).onFileSelected(file, name);
						} else {
							FileChooserCore.this.listeners.get(i).onFileSelected(file);
						}
					}
				}
			});
			alert.setNegativeButton(negButton, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					// Do nothing, automatically the dialog is going to be closed.
				}
			});

			// Show the dialog.
			alert.show();
		} else {
			// Notify to listeners.
			for(int i=0; i<FileChooserCore.this.listeners.size(); i++) {
				if(creation) {
					FileChooserCore.this.listeners.get(i).onFileSelected(file, name);
				} else {
					FileChooserCore.this.listeners.get(i).onFileSelected(file);
				}
			}
		}
	}

	// ----- Get and set methods ----- //

	/**
	 * Allows to define if a confirmation dialog must be show when selecting a file.
	 *
	 * @param show 'true' for show the confirmation dialog, 'false' for not show the dialog.
	 */
	public void setShowConfirmationOnSelect(boolean show) {
		this.showConfirmationOnSelect = show;
	}

	/**
	 * Allows to define if a confirmation dialog must be show when creating a file.
	 *
	 * @param show 'true' for show the confirmation dialog, 'false' for not show the dialog.
	 */
	public void setShowConfirmationOnCreate(boolean show) {
		this.showConfirmationOnCreate = show;
	}

	/**
	 * Allows to define if, in the title, must be show only the current folder's name or the full file's path..
	 *
	 * @param show 'true' for show the full path, 'false' for show only the name.
	 */
	public void setShowFullPathInTitle(boolean show) {
		this.showFullPathInTitle = show;
	}

        public void setSortLastModified(boolean sort){
            this.sortLastModified = sort;
        }


	/**
	 * Defines the value of the labels.
	 *
	 * @param label The labels.
	 */
	public void setLabels(FileChooserLabels labels) {
		this.labels = labels;

		// Verify if the buttons for add a file or select a folder has been modified.
		if(labels != null) {
			LinearLayout root = this.chooser.getRootLayout();

			if(labels.labelAddButton != null) {
				Button addButton = (Button) root.findViewById(R.id.buttonAdd);
				addButton.setText(labels.labelAddButton);
			}

			if(labels.labelSelectButton != null) {
				Button okButton = (Button) root.findViewById(R.id.buttonOk);
				okButton.setText(labels.labelSelectButton);
			}
		}
	}

	/**
	 * Set a regular expression to filter the files that can be selected.
	 *
	 * @param filter A regular expression.
	 */
	public void setFilter(String filter) {
		if(filter == null || filter.length() == 0 ) {
			this.filter = null;
		} else {
			this.filter = filter;
		}

		// Reload the list of files.
		this.loadFolder(this.currentFolder);
	}

	/**
	 * Defines if the chooser is going to be used to select folders, instead of files.
	 *
	 * @param folderMode 'true' for select folders or 'false' for select files.
	 */
	public void setFolderMode(boolean folderMode) {
		this.folderMode = folderMode;

		// Show or hide the 'Ok' button.
		updateButtonsLayout();

		// Reload the list of files.
		this.loadFolder(this.currentFolder);
	}

	/**
	 * Defines if the user can create files, instead of only select files.
	 *
	 * @param canCreate 'true' if the user can create files or 'false' if it can only select them.
	 */
	public void setCanCreateFiles(boolean canCreate) {
		this.canCreateFiles = canCreate;

		// Show or hide the 'Add' button.
		updateButtonsLayout();
	}

	/**
	 * Defines if only the files that can be selected (they pass the filter) must be show.
	 *
	 * @param show 'true' if only the files that can be selected must be show or 'false' if all the files must be show.
	 */
	public void setShowOnlySelectable(boolean show) {
		this.showOnlySelectable = show;

		// Reload the list of files.
		this.loadFolder(this.currentFolder);
	}

	/**
	 * Returns the current folder.
	 *
	 * @return The current folder.
	 */
	public File getCurrentFolder() {
		return this.currentFolder;
	}

	// ----- Miscellaneous methods ----- //

	/**
	 * Changes the height of the layout for the buttons, according if the buttons are visible or not.
	 */
	private void updateButtonsLayout() {
		// Get the buttons layout.
		LinearLayout root = this.chooser.getRootLayout();
		LinearLayout buttonsLayout = (LinearLayout) root.findViewById(R.id.linearLayoutButtons);

		// Verify if the 'Add' button is visible or not.
		View addButton = root.findViewById(R.id.buttonAdd);
		addButton.setVisibility(this.canCreateFiles? View.VISIBLE : View.INVISIBLE);
		addButton.getLayoutParams().width = this.canCreateFiles? ViewGroup.LayoutParams.MATCH_PARENT : 0;

		// Verify if the 'Ok' button is visible or not.
		View okButton = root.findViewById(R.id.buttonOk);
		okButton.setVisibility(this.folderMode? View.VISIBLE : View.INVISIBLE);
		okButton.getLayoutParams().width = this.folderMode? ViewGroup.LayoutParams.MATCH_PARENT : 0;

		// If both buttons are invisible, hide the layout.
		ViewGroup.LayoutParams params = buttonsLayout.getLayoutParams();
		if(this.canCreateFiles || this.folderMode) {
			// Show the layout.
			params.height = ViewGroup.LayoutParams.WRAP_CONTENT;

			// If only the 'Ok' button is visible, put him first. Otherwise, put 'Add' first.
			buttonsLayout.removeAllViews();
			if(this.folderMode && !this.canCreateFiles) {
				buttonsLayout.addView(okButton);
				buttonsLayout.addView(addButton);
			} else {
				buttonsLayout.addView(addButton);
				buttonsLayout.addView(okButton);
			}
		} else {
			// Hide the layout.
			params.height = 0;
		}
	}

	/**
	 * Loads all the files of the SD card root.
	 */
	public void loadFolder() {
		this.loadFolder(defaultFolder);
	}

	/**
	 * Loads all the files of a folder in the file chooser.
	 *
	 * If no path is specified ('folderPath' is null) the root folder of the SD card is going to be used.
	 *
	 * @param folderPath The folder's path.
	 */
	public void loadFolder(String folderPath) {
		// Get the file path.
		File path = null;
		if(folderPath != null && folderPath.length() > 0) {
			path = new File(folderPath);
		}

		this.loadFolder(path);
	}

	/**
	 * Loads all the files of a folder in the file chooser.
	 *
	 * If no path is specified ('folder' is null) the root folder of the SD card is going to be used.
	 *
	 * @param folder The folder.
	 */
	public void loadFolder(File folder) {
            //Log.i("DEBUGGER_TAG", "LoadFolder: " + folder);

		// Remove previous files.
		LinearLayout root = this.chooser.getRootLayout();
		LinearLayout layout = (LinearLayout) root.findViewById(R.id.linearLayoutFiles);
		layout.removeAllViews();

		// Get the file path.
		if(folder == null || !folder.exists()) {
			if(defaultFolder != null) {
				this.currentFolder = defaultFolder;
			} else {
				this.currentFolder = Environment.getExternalStorageDirectory();
			}
		} else {
			this.currentFolder = folder;
		}

		// Verify if the path exists.
		if(this.currentFolder.exists() && layout != null) {
                    //Log.i("DEBUGGER_TAG", "Exists");

			List<FileItem> fileItems = new LinkedList<FileItem>();

			// Add the parent folder.
			if(this.currentFolder.getParent() != null) {
				File parent = new File(this.currentFolder.getParent());
				if(parent.exists()) {
					fileItems.add(new FileItem(this.chooser.getContext(), parent, ".."));
				}
			}

			// Verify if the file is a directory.
			if(this.currentFolder.isDirectory()) {
				// Get the folder's files.

                                File[] fileList = this.currentFolder.listFiles();
				if(fileList != null) {


					// Order the files alphabetically and separating folders from files.
                                        Arrays.sort(fileList, new Comparator<File>() {
                                                public int compare(File file1, File file2) {
                                                        if(file1 != null && file2 != null) {
                                                                if(file1.isDirectory() && (!file2.isDirectory())) return -1;
                                                                if(file2.isDirectory() && (!file1.isDirectory())) return 1;
                                                                return file1.getName().compareTo(file2.getName());
                                                        }
                                                        return 0;
                                                }
                                        });

                                        // Order the files by reverse date.
                                        if(sortLastModified){
                                            Arrays.sort(fileList, new Comparator<File>() {
                                                public int compare(File file1, File file2) {
                                                        if(file1 != null && file2 != null) {
                                                                if(file1.isDirectory() && (!file2.isDirectory())) return -1;
                                                                if(file2.isDirectory() && (!file1.isDirectory())) return 1;
                                                                int rv = 0;
                                                                if(file1.lastModified() < file2.lastModified())
                                                                    rv = 1;
                                                                else
                                                                    rv = -1;
                                                                return rv;
                                                        }
                                                        return 0;
                                                }
                                            });
                                        }

					// Iterate all the files in the folder.
					for(int i=0; i<fileList.length; i++) {
						// Verify if file can be selected (is a directory or folder mode is not activated and the file pass the filter, if defined).
						boolean selectable = true;
						if(!fileList[i].isDirectory()) {
							selectable = !this.folderMode && (this.filter == null || fileList[i].getName().matches(this.filter));
						}

						// Verify if the file must be show.
						if(selectable || !this.showOnlySelectable) {
							// Create the file item and add it to the list.
							FileItem fileItem = new FileItem(this.chooser.getContext(), fileList[i]);
							fileItem.setSelectable(selectable);
							fileItems.add(fileItem);
						}
					}
				}

				// Set the name of the current folder.
				String currentFolderName = this.showFullPathInTitle? this.currentFolder.getPath() : this.currentFolder.getName();
				this.chooser.setCurrentFolderName(currentFolderName);


			} else {
				// The file is not a folder, add only this file.
				fileItems.add(new FileItem(this.chooser.getContext(), this.currentFolder));
			}


			// Add click listener and add the FileItem objects to the layout.
			for(int i=0; i<fileItems.size(); i++) {
				fileItems.get(i).addListener(this.fileItemClickListener);
				layout.addView(fileItems.get(i));
			}

			// Refresh default folder.
			defaultFolder = this.currentFolder;
		}

        updateSourceGUI(this.currentFolder);


    }

        public void loadFolderA(File folder) {

            //Log.i("DEBUGGER_TAG", "LoadFolderA");

                // Remove previous files.
                LinearLayout root = this.chooser.getRootLayout();
                LinearLayout layout = (LinearLayout) root.findViewById(R.id.linearLayoutFiles);
                m_progressBar = (ProgressBar) root.findViewById(R.id.progressBar);
                layout.removeAllViews();

                // Get the file path.
                if(folder == null || !folder.exists()) {
                        if(defaultFolder != null) {
                                this.currentFolder = defaultFolder;
                        } else {
                                this.currentFolder = Environment.getExternalStorageDirectory();
                        }
                } else {
                        this.currentFolder = folder;
                }

                // Verify if the path exists.
                if(this.currentFolder.exists() && layout != null) {
                   // Log.i("DEBUGGER_TAG", "Exists");
                        List<FileItem> fileItems = new LinkedList<FileItem>();

                        // Add the parent folder.
                        if(this.currentFolder.getParent() != null) {
                                File parent = new File(this.currentFolder.getParent());
                                if(parent.exists()) {
                                        fileItems.add(new FileItem(this.chooser.getContext(), parent, ".."));
                                }
                        }

                        // Verify if the file is a directory.
                        if(this.currentFolder.isDirectory()) {



                                m_task = new GetFilesTask(this);
                                m_task.execute(this);

                        } else {
                                // The file is not a folder, add only this file.
//				fileItems.add(new FileItem(this.chooser.getContext(), this.currentFolder));
                        }

                        // Set the name of the current folder.
                        String currentFolderName = this.showFullPathInTitle? this.currentFolder.getPath() : this.currentFolder.getName();
                        this.chooser.setCurrentFolderName(currentFolderName);

                }
        }

    private void updateSourceGUI( File folder){
        if(null == folder)
            return;

        // Update the GUI for SDCard, etc...
        LinearLayout root = this.chooser.getRootLayout();

        Button sdcardButton = (Button) root.findViewById(R.id.buttonSDCard);
        Button deviceButton = (Button) root.findViewById(R.id.buttonDevice);
        String SD = getExtSdCardFolder(folder);

        if((null != m_sdcardDir) && (null != folder)){
            if(null != SD){
                sdcardButton.setAlpha((float)1.0);
                deviceButton.setAlpha((float) 0.2);
            }
            else {
                sdcardButton.setAlpha((float)0.2);
                deviceButton.setAlpha((float) 1.0);
            }

        }
/*
            String[] sda = m_sdcardDir.split("/");
            String[] target = folder.getAbsolutePath().split("/");

            if( (sda.length >= 3 ) && (target.length >= 3)){
                if ( (sda[1].equals(target[1])) && (sda[2].equals(target[2]))){
                    sdcardButton.setAlpha((float)1.0);
                    deviceButton.setAlpha((float) 0.2);
                }
                else {
                    sdcardButton.setAlpha((float)0.2);
                    deviceButton.setAlpha((float) 1.0);
                }
            }
        }
*/
        else {
            deviceButton.setAlpha((float) 1.0);
        }

    }

    private void triggerStorageAccessFramework() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        FileChooserActivity act = (FileChooserActivity)chooser;
        if(null != act)
            act.startActivityForResult(intent, 43);
    }


    private boolean checkStoragePermissions(){
        FileChooserActivity act = (FileChooserActivity)chooser;
        if(null != act) {

            List<UriPermission> list = act.getContentResolver().getPersistedUriPermissions();
            for (int i = 0; i < list.size(); i++) {

                String perm = list.get(i).getUri().toString();

                //   if (list.get(i).getUri() == myUri && list.get(i).isWritePermission()) {
             //       return true;


            }
        }
        return false;
    }

    /**
     * Get a list of external SD card paths. (Kitkat or higher.)
     *
     * @return A list of external SD card paths.
     */
    private String[] getExtSdCardPaths() {
        List<String> paths = new ArrayList();


        if( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {        // KITKAT = 19/20
            for (File file : chooser.getContext().getExternalFilesDirs("external")) {
                if (file != null && !file.equals(chooser.getContext().getExternalFilesDir("external"))) {
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
        }
        else{
            if(null != m_sdcardDir)
                paths.add(m_sdcardDir);
        }

        return paths.toArray(new String[paths.size()]);
    }

    public String getExtSdCardFolder(final File file) {
        String[] extSdPaths = getExtSdCardPaths();

        try {
            String file_path = file.getCanonicalPath();
            if(file_path.length() == 0)
                return null;

            for (int i = 0; i < extSdPaths.length; i++) {
                if(extSdPaths[i].length() != 0){
                    if (file.getCanonicalPath().startsWith(extSdPaths[i])) {
                        return extSdPaths[i];
                    }
                }
            }
        }
        catch (IOException e) {
            return null;
        }
        return null;
    }

}
