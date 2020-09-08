
require 'socket'
starttime = Process.clock_gettime(Process::CLOCK_MONOTONIC)
s = TCPSocket.new 'localhost', 1234

s.write("join chat #{ARGV[0]}\n")

s.write("Testing #{ARGV[0]}\n")

s.each_line do |line|
    puts line
end
s.close
endtime = Process.clock_gettime(Process::CLOCK_MONOTONIC)
elapsed = endtime - starttime
puts "Elapsed: #{elapsed}"
