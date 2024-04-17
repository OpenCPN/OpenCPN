/**
\page plugincomms Handling Communications Messages in Plugins


From 5.10 a new ObsListener class offers a simple way to listen to messages.
The simplified approach:

  1.    Declare a ObsListener listener object
  2.    Declare a method which handles the message.
  3.    Initiate the listener to invoke the method when message arrives.

The ObsListener object normally lives in the class context, like:

        class Dlg : public ShipDriverBase {
        public:
            ...
        private:
            ObsListener  listener;
            ...

The method is declared something like below.

       static void HandleGPGA(ObservedEvt ev) {
            NMEA0183Id id("GPGGA");
            std::string payload = GetN0183Payload(id, ev)
            ...

There are other ids for NMEA2000 and SignalK messages available.
Likewise, there are similar methods to access the payload.
See the ocpn_plugin.h header file.

To invoke this method when a GPGGA message arrives, initiate the ObsListener like:

        listener.Init(NMEA0183Id("GPGGA"),
                      [&](ObservedEvt ev) { HandleGPGA(ev); });

The last line is a lambda expression which could be used to much more than to just
invoke a method, see Epilog: Using the lambda below.


\subsection epilog Epilog: Using the lambda

The last step above was
`Bind(EVT_SHIPDRIVER [](wxCommandEvent ev) { HandleGPGA(ev); })`.
The part between the curly braces is actually a C++ lambda expression.
There is no need to dive into this to get something running, but it offers
far larger possibilities than to just call a function.
Actually, if written like
 `Bind(EVT_SHIPDRIVER, [&](wxCommandEvent ev) { ... })` any code could
be written between the braces.

The interesting part here is that the `[&]` prefix makes this code "see"
anything defined in the plugin.
This is a convenient way to access plugin variables in the handler,
something which otherwise is a problem.

To get the feeling one need to experiment.
But then again, C++ lambdas is a complex step which is not necessary
to get something running.
**/
