# Python modules
import time
import signal

# 3rd party modules
import posix_ipc

QUEUE_NAME = "/synthqueue"

#MY_SIGNAL = signal.SIGUSR1


#def handle_signal(signal_number, stack_frame):
#   message, priority = mq.receive()
#    
#    print ("Ding! Message with priority %d received: %s" % (priority, message))
    


# Create the message queue.
#mq = posix_ipc.MessageQueue(QUEUE_NAME, posix_ipc.O_CREX)
mq = posix_ipc.MessageQueue(QUEUE_NAME, 0)

# Request notifications
#mq.request_notification(MY_SIGNAL)

# Register my signal handler 
#signal.signal(MY_SIGNAL, handle_signal)

# Get user input and send it to the queue.
print ("Sending message 1...")
mq.send(b"AAA")

# The signal fires almost instantly, but if I don't pause at least 
# briefly then the main thread may exit before the notification fires.
#print ("Sleeping for one second to allow the notification to happen.")
time.sleep(2)
print ("Sending message 2...")
mq.send(b"GGG")

print ("Closing message queue...")
mq.close()

#print ("Destroying the message queue.")
#mq.close()
# I could call simply mq.unlink() here but in order to demonstrate 
# unlinking at the module level I'll do it that way.
#posix_ipc.unlink_message_queue(utils.QUEUE_NAME)

