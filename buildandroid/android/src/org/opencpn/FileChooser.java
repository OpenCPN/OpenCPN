package org.opencpn;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.app.ListActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.Toast;
import org.opencpn.opencpn.R;


public class FileChooser extends ListActivity {

    private File currentDir;
    private FileArrayAdapter adapter;

    public static String m_selected;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.xml.file_chooser_layout);

        currentDir = new File("/sdcard/");
        fill(currentDir);
    }
    private void fill(File f)
    {
        File[]dirs = f.listFiles();
         this.setTitle("Current Dir: "+f.getName());
         List<Option>dir = new ArrayList<Option>();
         List<Option>fls = new ArrayList<Option>();
         try{
             for(File ff: dirs)
             {
                if(ff.isDirectory())
                    dir.add(new Option(ff.getName(),"Folder",ff.getAbsolutePath()));
//                else
//                {
//                    fls.add(new Option(ff.getName(),"File Size: "+ff.length(),ff.getAbsolutePath()));
//                }
             }
         }catch(Exception e)
         {

         }
         Collections.sort(dir);
         Collections.sort(fls);
         dir.addAll(fls);
         if(!f.getName().equalsIgnoreCase("sdcard"))
             dir.add(0,new Option("..","Parent Directory",f.getParent()));
         adapter = new FileArrayAdapter(FileChooser.this,R.xml.file_view,dir);
         this.setListAdapter(adapter);
    }
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        // TODO Auto-generated method stub
        super.onListItemClick(l, v, position, id);
        Option o = adapter.getItem(position);
        m_selected = o.getPath();

        if(o.getData().equalsIgnoreCase("folder")||o.getData().equalsIgnoreCase("parent directory")){
                currentDir = new File(o.getPath());
                fill(currentDir);
                onDirClick(o);

        }
        else
        {
            onFileClick(o);
        }
    }
    private void onFileClick(Option o)
    {
        Toast.makeText(this, "File Clicked: "+o.getName(), Toast.LENGTH_SHORT).show();
    }

    private void onDirClick(Option o)
    {
        Toast.makeText(this, "Dir Clicked: "+m_selected, Toast.LENGTH_SHORT).show();
    }

    public void addDirectory(View vw)
    {
        Toast.makeText(this, "Dir Clicked a: "+m_selected, Toast.LENGTH_SHORT).show();

        Bundle b = new Bundle();
        b.putString("SelectedDir", m_selected);
        Intent i = getIntent(); //gets the intent that called this intent
        i.putExtras(b);
        setResult(FileChooser.RESULT_OK, i);
        finish();
    }

}
