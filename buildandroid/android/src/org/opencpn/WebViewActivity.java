
package org.opencpn;

import android.app.Activity;
import android.os.Bundle;
import android.webkit.WebView;
import org.opencpn.opencpn.R;

public class WebViewActivity extends Activity {

        private WebView webView;

        public void onCreate(Bundle savedInstanceState) {
                super.onCreate(savedInstanceState);
                setContentView(R.layout.webview);

                webView = (WebView) findViewById(R.id.webView1);
                webView.getSettings().setJavaScriptEnabled(true);

                //webView.loadUrl("file://localhost/storage/sdcard0/doc/help_en_US.html");  // this worked, after manual copy
                webView.loadUrl("file:///android_asset/doc/doc/help_en_US.html");
        }

}
