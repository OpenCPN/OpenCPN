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

package ar.com.daidalos.afiledialog.view;

import java.io.File;
import java.util.LinkedList;
import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import ar.com.daidalos.afiledialog.R;

/**
 * This class is used to represents the files that can be selected by the user. 
 */
public class FileItem extends LinearLayout {

	// ----- Attributes ----- //
	
	/**
	 * The file which is represented by this item.
	 */
	private File file;
	
	/**
	 * The image in which show the file's icon.
	 */
	private ImageView icon;
	
	/**
	 * The label in which show the file's name.
	 */
	private TextView label;
	
	/**
	 * A boolean indicating if the item can be selected.
	 */
	private boolean selectable;
	
	/**
	 * The listeners for the click event.
	 */
	private List<FileItem.OnFileClickListener> listeners;
	
	// ----- Constructor ----- //
	
	/**
	 * The class main constructor.
	 * 
	 * @param context The application's context.
	 */
	public FileItem(Context context) {
		super(context);
		
		// Define the layout.
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		inflater.inflate(R.layout.daidalos_file_item, this, true);
		
		// Initialize attributes.
		this.file = null;
		this.selectable = true;
		this.icon = (ImageView) this.findViewById(R.id.imageViewIcon);
		this.label = (TextView) this.findViewById(R.id.textViewLabel);
		this.listeners = new LinkedList<FileItem.OnFileClickListener>();
		
		// Add a listener for the click event.
		this.setOnClickListener(this.clickListener);
	}
	
	/**
	 * A class constructor.
	 * 
	 * @param context The application's context.
	 * @param file The file represented by this item
	 */
	public FileItem(Context context, File file) {
		this(context);
		
		// Set the file.
		this.setFile(file);
	}
	
	/**
	 * A class constructor.
	 * 
	 * @param context The application's context.
	 * @param file The file represented by this item.
	 * @param label The label of this item.
	 */
	public FileItem(Context context, File file, String label) {
		this(context, file);
		
		// Set the label.
		this.setLabel(label);
	}
	
	// ----- Get() and Set() methods ----- //
	
	/**
	 * Defines the file represented by this item.
	 * 
	 * @param file A file.
	 */
	public void setFile(File file) {
		if(file != null) {
			this.file = file;
			
			// Replace the label by the file's name.
			this.setLabel(file.getName());
			
			// Change the icon, depending if the file is a folder or not.
			this.updateIcon();
		}
	}
	
	/**
	 * Returns the file represented by this item. 
	 * 
	 * @return A file.
	 */
	public File getFile() {
		return this.file;
	}
	
	/**
	 * Changes the label of this item, which by default is the file's name.
	 * 
	 * This method must be called after invoking the method setFile(), otherwise 
	 * the label is going to be overwritten with the file's name.
	 * 
	 * @param label A string value.
	 */
	public void setLabel(String label) {
		// Verify if 'label' is not null.
		if(label == null) label = "";
		
		// Change the label.
		this.label.setText(label);
	}
	
	/**
	 * Verifies if the item can be selected.
	 * 
	 * @return 'true' if the item can be selected, 'false' if not.
	 */
	public boolean isSelectable() {
		return this.selectable;
	}
	
	/**
	 * Defines if the item can be selected or not.
	 * 
	 * @param selectable 'true' if the item can be selected, 'false' if not.
	 */
	public void setSelectable(boolean selectable) {
		// Save the value.
		this.selectable = selectable;
		
		// Update the icon.
		this.updateIcon();
	}
	
	// ----- Miscellaneous methods ----- //
	
	/**
	 * Updates the icon according to if the file is a folder and if it can be selected.
	 */
	private void updateIcon() {
		// Define the icon.
		int icon = R.drawable.document_gray;
		if(this.selectable) {
			icon = (this.file != null && file.isDirectory())? R.drawable.folder : R.drawable.document;
		}

		// Set the icon.
		this.icon.setImageDrawable(getResources().getDrawable( icon ));

		// Change the color of the text.
		if(icon != R.drawable.document_gray) {
			this.label.setTextColor(getResources().getColor(R.color.daidalos_active_file));
		} else {
			this.label.setTextColor(getResources().getColor(R.color.daidalos_inactive_file));
		}
	}
	
	// ----- Events ----- //
	
	/**
	 * Listener for the click event.
	 */
	private View.OnClickListener clickListener = new View.OnClickListener() {
		
		public void onClick(View v) {
			// Verify if the item can be selected.
			if(FileItem.this.selectable) {
				// Call the listeners.
				for(int i=0; i<FileItem.this.listeners.size(); i++) {
					FileItem.this.listeners.get(i).onClick(FileItem.this);
				}
			}
		}
	};
	
	/**
	 * Add a listener for the click event.
	 * 
	 * @param listener The listener to add.
	 */
	public void addListener(FileItem.OnFileClickListener listener) {
		this.listeners.add(listener);
	}
	
	/**
	 * Removes a listener for the click event.
	 * 
	 * @param listener The listener to remove.
	 */
	public void removeListener(FileItem.OnFileClickListener listener) {
		this.listeners.remove(listener);
	}
	
	/**
	 * Removes all the listeners for the click event.
	 */
	public void removeAllListeners() {
		this.listeners.clear();
	}
	
	/**
	 * Interface definition for a callback to be invoked when a FileItem is clicked. 
	 */
	public interface OnFileClickListener {
		/**
		 * Called when a FileItem has been clicked.
		 * 
		 * @param source The source of the event.
		 */
		void onClick(FileItem source);
	}
}
