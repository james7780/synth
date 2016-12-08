import os
import pygame
import time
import random

# Define some colours
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)
YELLOW = (255, 255, 0)
PURPLE = (255, 0, 255)


def initPygameVideo() :
	global screen
	"Ininitializes a new pygame screen using the framebuffer"
	# Based on "Python GUI in Linux frame buffer"
	# http://www.karoltomala.com/blog/?p=679
	disp_no = os.getenv("DISPLAY")
	if disp_no:
		print "I'm running under X display = {0}".format(disp_no)
        
	# Check which frame buffer drivers are available
	# Start with fbcon since directfb hangs with composite output
	drivers = ['fbcon'] #, 'directfb', 'svgalib']
	found = False
	for driver in drivers:
		# Make sure that SDL_VIDEODRIVER is set
		if not os.getenv('SDL_VIDEODRIVER'):
			os.putenv('SDL_VIDEODRIVER', driver)
		try:
			pygame.display.init()
		except pygame.error:
			print 'Driver: {0} failed.'.format(driver)
			continue
		found = True
		break
    
	if not found:
		raise Exception('No suitable video driver found!')
	
	size = (pygame.display.Info().current_w, pygame.display.Info().current_h)
	print "Framebuffer size: %d x %d" % (size[0], size[1])
	screen = pygame.display.set_mode(size, pygame.FULLSCREEN)
	# Clear the screen to start
	screen.fill(GREEN)        
	# Initialise font support
	pygame.font.init()
	# Render the screen
	pygame.display.update()
 
def test() :
	# Fill the screen with red
	screen.fill(RED)
	# Update the display
	pygame.display.update()
 
## Create an instance of the PyScope class
#scope = pyscope()
#scope.test()
#time.sleep(10)

initPygameVideo()
test()
time.sleep(1)

# Clock is fro tracking framerate
clock = pygame.time.Clock()

font = pygame.font.Font(None, 64)

posx = 100

# ------------ MAIN LOOP ---------------
done = False;
while not done :
	# HAndle events (mouse / keys)
	for event in pygame.event.get() :
		#if event.type == pygame.QUIT :
		# pygame.QUIT not valid for fullscreen mode
		if event.type == pygame.KEYDOWN :
			done = True
			
	# logic goses here
	
	
	# Drawing code here
	
	
	# do something
	screen.fill(GREEN)
	pygame.draw.rect(screen, WHITE, (10, 10, 50, 50), 3)
	
	fontSurf = font.render("Hi James", True, (255, 255, 255, 0))
	screen.blit(fontSurf, (posx, 100))
	posx += 1
	
	pygame.display.flip()
	
	# fps limiter
	clock.tick(60)
	
	
	
pygame.quit()




