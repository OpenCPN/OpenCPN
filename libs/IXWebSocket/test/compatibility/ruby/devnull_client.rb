#
# $ ruby --version
# ruby 2.6.3p62 (2019-04-16 revision 67580) [universal.x86_64-darwin19]
# 
# Install a gem locally by setting GEM_HOME
# https://stackoverflow.com/questions/486995/ruby-equivalent-of-virtualenv
# export GEM_HOME=$HOME/local/gems
# bundle install faye-websocket
#
# In a different terminal, start a push server:
# $ ws push_server -q
#
# $ ruby devnull_client.rb
# [:open]
# Connected to server
# messages received per second: 115926
# messages received per second: 119156
# messages received per second: 119156
# messages received per second: 119157
# messages received per second: 119156
# messages received per second: 119156
# messages received per second: 119157
# messages received per second: 119156
# messages received per second: 119156
# messages received per second: 119157
# messages received per second: 119156
# messages received per second: 119157
# messages received per second: 119156
# ^C[:close, 1006, ""]
# 
require 'faye/websocket'
require 'eventmachine'

EM.run {
  ws = Faye::WebSocket::Client.new('ws://127.0.0.1:8008')

  counter = 0

  EM.add_periodic_timer(1) do
    print "messages received per second: #{counter}\n"
    counter = 0  # reset counter
  end

  ws.on :open do |event|
    p [:open]
    print "Connected to server\n"
  end

  ws.on :message do |event|
    # Uncomment the next line to validate that we receive something correct
    # p [:message, event.data]
    counter += 1
  end

  ws.on :close do |event|
    p [:close, event.code, event.reason]
    ws = nil
  end
}
