package org.opencpn;

import android.content.Context;
import android.util.AttributeSet;
import android.preference.DialogPreference;
import android.view.View;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ArrayAdapter;
import android.widget.AdapterView;
import android.util.Log;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.opencpn.opencpn.R;





public class ChartFilesDialogPreference extends DialogPreference
{
    public ChartFilesDialogPreference(Context oContext, AttributeSet attrs)
    {
        super(oContext, attrs);
        m_context = oContext;

//        setDialogLayoutResource(R.xml.chart_files_dialog_layout);

        setPositiveButtonText(android.R.string.ok);
        setNegativeButtonText(android.R.string.cancel);

        setDialogIcon(null);

        setPersistent(false);
    }

    public String getSelected(){
        return m_selectedItem;
    }

    @Override
    protected View onCreateDialogView() {

        LayoutInflater inflater = (LayoutInflater)m_context.getSystemService
              (Context.LAYOUT_INFLATER_SERVICE);

        View vw = inflater.inflate(R.layout.chart_files_dialog_layout1, null);
        m_vw = vw;

        Button btn = (Button) m_vw.findViewById(R.id.buttonRemove);
        btn.setEnabled(false);

        String settings = OCPNSettingsActivity.getSettingsString();
        Log.i("DEBUGGER_TAG", "ChartFilesDialogPreference");
       // Log.i("DEBUGGER_TAG", settings);


        m_lv = (ListView) vw.findViewById(R.id.chartdirlist);

        String[] values = new String[] { "Android", "iPhone", "WindowsMobile",
            "Blackberry", "WebOS", "Ubuntu", "Windows7", "Max OS X",
            "Linux", "OS/2", "Ubuntu", "Windows7", "Max OS X", "Linux",
            "OS/2", "Ubuntu", "Windows7", "Max OS X", "Linux", "OS/2",
            "Android", "iPhone", "WindowsMobile" };

//        final ArrayList<String> list = new ArrayList<String>();
//        for (int i = 0; i < values.length; ++i) {
//          list.add(values[i]);
//        }

        m_chartDirList = OCPNSettingsActivity.getChartDirList();

        m_adapter = new StableArrayAdapter(m_context,
                R.layout.listitem, m_chartDirList);

        m_lv.setAdapter(m_adapter);

        m_lv.setSelector(R.drawable.selector);

        m_lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {

           @Override
           public void onItemClick(AdapterView<?> parent, final View view,
               int position, long id) {
               Button btn = (Button) m_vw.findViewById(R.id.buttonRemove);
               btn.setEnabled(true);

               // how to change the selected background color
               //http://www.michenux.net/android-listview-highlight-selected-item-387.html
             final String item = (String) parent.getItemAtPosition(position);
             m_selectedItem = item;
             view.animate().setDuration(2000).alpha(0)
                 .withEndAction(new Runnable() {
                   @Override
                   public void run() {
                  //   m_chartDirList.remove(item);
                     m_adapter.notifyDataSetChanged();
                     view.setAlpha(1);
                   }
                 });
           }

         });

        return vw;
     }


//     http://stackoverflow.com/questions/4505845/concise-way-of-writing-new-dialogpreference-classes

     public void addDirectory(View view) {
         Log.i("DEBUGGER_TAG", "addDirectoryClick");

         // Do something in response to button click
     }


//     Button button = (Button) findViewById(R.id.button_send);
//     button.setOnClickListener(new View.OnClickListener() {
//         public void onClick(View v) {
             // Do something in response to button click
//         }
//     });




     private class StableArrayAdapter extends ArrayAdapter<String> {

         HashMap<String, Integer> mIdMap = new HashMap<String, Integer>();

         public StableArrayAdapter(Context context, int textViewResourceId,
             List<String> objects) {
           super(context, textViewResourceId, objects);
           for (int i = 0; i < objects.size(); ++i) {
             mIdMap.put(objects.get(i), i);
           }
         }

         @Override
         public long getItemId(int position) {
           String item = getItem(position);
           return mIdMap.get(item);
         }

         @Override
         public boolean hasStableIds() {
           return true;
         }

         @Override
         public View getView(int position, View convertView, ViewGroup parent) {
             // Get the data item for this position
             String user = getItem(position);
             // Check if an existing view is being reused, otherwise inflate the view
             if (convertView == null) {
                 convertView = LayoutInflater.from(getContext()).inflate(R.layout.listitem, parent, false);
             }

             TextView Name = (TextView) convertView.findViewById(R.id.listItem);
             Name.setText(user);

             // Return the completed view to render on screen
             return convertView;
         }

     }


       public void updateListView(){
           m_chartDirList = OCPNSettingsActivity.getChartDirList();

           m_adapter = new StableArrayAdapter(m_context,
               android.R.layout.simple_list_item_1, m_chartDirList);
           m_lv.setAdapter(m_adapter);

           Button btn = (Button) m_vw.findViewById(R.id.buttonRemove);
           btn.setEnabled(false);

//           m_adapter.notifyDataSetChanged();
//           view.setAlpha(1);
       }




       private Context m_context;
       public StableArrayAdapter m_adapter;
       private ArrayList<String> m_chartDirList;
       private ListView m_lv;
       private View m_vw;
       private String m_selectedItem;

}








