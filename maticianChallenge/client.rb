
require 'socket'
#starttime = Process.clock_gettime(Process::CLOCK_MONOTONIC)
s = TCPSocket.new 'localhost', 1234

$i = 0
$j = 1999500
$testString = ""
s.write("join chat #{ARGV[0]}\n")

#s.write("Testing #{ARGV[0]}\n"

while $i < $j do
    $testString += "."
    $i += 1
end
 $testString += "\r"
  $testString += "\n"
   $testString +="z"
$testString += "\n";
s.write($testString)

s.each_line do |line|
    puts line
end
s.close
#endtime = Process.clock_gettime(Process::CLOCK_MONOTONIC)
#elapsed = endtime - starttime
#puts "Elapsed: #{elapsed}"
