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

import java.io.Serializable;

/**
 * Instances of this classes are used to re-define the value of the labels of a file chooser. 
 * 
 * If an attribute is set to null, then the default value is going to be used.
 */
public class FileChooserLabels implements Serializable {
	
	/**
	 * Static field required by the interface Serializable.
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * Default's constructor.
	 */
	public FileChooserLabels() {
		this.labelAddButton = null;
		this.labelSelectButton = null;
		this.messageConfirmSelection = null;
		this.messageConfirmCreation = null;
		this.labelConfirmYesButton = null;
		this.labelConfirmNoButton = null;
		this.createFileDialogTitle = null;
		this.createFileDialogTitle = null;
		this.createFileDialogAcceptButton = null;
		this.createFileDialogCancelButton = null;
	}
	
	/**
	 * The label for the button used to create a file or a folder.
	 */
	public String labelAddButton;
	
	/**
	 * The label for the button for select the current folder (when using the file chooser for select folders).
	 */
	public String labelSelectButton;
	
	/**
	 * The message displayed by the confirmation dialog, when selecting a file.
	 * 
	 * In this string, the character sequence '$file_name' is going to be replace by the file's name.
	 */
	public String messageConfirmSelection;
	
	/**
	 * The message displayed by the confirmation dialog, when creating a file.
	 * 
	 * In this string, the character sequence '$file_name' is going to be replace by the file's name.
	 */
	public String messageConfirmCreation;
	
	/**
	 * The label for the 'yes' button when confirming the selection o creation of a file.
	 */
	public String labelConfirmYesButton;
	
	/**
	 * The label for the 'no' button when confirming the selection o creation of a file.
	 */
	public String labelConfirmNoButton;
	
	/**
	 * The title of the dialog for create a file.
	 */
	public String createFileDialogTitle;
	
	/**
	 * The message of the dialog for create a file.
	 */
	public String createFileDialogMessage;
	
	/**
	 * The label of the 'accept' button in the dialog for create a file.
	 */
	public String createFileDialogAcceptButton;
	
	/**
	 * The label of the 'cancel' button in the dialog for create a file.
	 */
	public String createFileDialogCancelButton;
}
