
package org.opencpn;

import android.app.Activity;
import android.os.Bundle;
import android.webkit.WebView;
import org.opencpn.opencpn.R;

public class WebViewActivity extends Activity {

        private WebView webView;

        public static String SELECTED_URL = "SelectedURL";

        public void onCreate(Bundle savedInstanceState) {
                super.onCreate(savedInstanceState);
                setContentView(R.layout.webview);

                webView = (WebView) findViewById(R.id.webView1);
                webView.getSettings().setJavaScriptEnabled(true);

                String url = "";
                Bundle bundle = getIntent().getExtras();
                if(bundle != null){
                    url = (String)bundle.get(WebViewActivity.SELECTED_URL);
                }


                //webView.loadUrl("file://localhost/storage/sdcard0/doc/help_en_US.html");  // this worked, after manual copy
                //webView.loadUrl("file:///android_asset/doc/doc/help_en_US.html");
                webView.loadUrl(url);
        }

}
