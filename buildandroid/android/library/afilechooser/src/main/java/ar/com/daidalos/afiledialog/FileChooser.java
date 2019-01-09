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

import android.content.Context;
import android.widget.LinearLayout;

/**
 * This interface defines all the methods that a file chooser must implement, in order to being able to make use of the class FileChooserUtils.
 */
interface FileChooser {

	/**
	 * Gets the root of the layout 'file_chooser.xml'.
	 * 
	 * @return A linear layout.
	 */
	LinearLayout getRootLayout();
	
	/**
	 * Set the name of the current folder.
	 * 
	 * @param name The current folder's name.
	 */
	void setCurrentFolderName(String name);
	
	/**
	 * Returns the current context of the file chooser.
	 * 
	 * @return The current context.
	 */
	Context getContext();
}
