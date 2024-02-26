-- Define materials for different parts of the head
local white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
local blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
local black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
local red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)

-- Create the main node for the head
local head = gr.node('node')
head:translate(0.0, 0.0, -7.0) -- Translate the head to the right position

-- Main sphere for the head
local head_sphere = gr.mesh('sphere', 'head_sphere')
head_sphere:set_material(blue)
head_sphere:scale(1.5, 1.5, 1.5) -- Scale the head to make it oval if needed
head:add_child(head_sphere)

-- Left eye
local left_eye = gr.mesh('sphere', 'left_eye')
left_eye:set_material(white)
left_eye:scale(0.5, 0.5, 0.5) -- Scale to make the eye smaller
left_eye:translate(-0.4, 0.6, 1.0) -- Position the eye on the face
head:add_child(left_eye)

-- Right eye, similar to the left eye
local right_eye = gr.mesh('sphere', 'right_eye')
right_eye:set_material(white)
right_eye:scale(0.5, 0.5, 0.5)
right_eye:translate(0.4, 0.6, 1.0)
head:add_child(right_eye)

-- Pupils can be created using smaller, black spheres or as part of the eye texture if textures are supported
local left_pupil = gr.mesh('sphere', 'left_pupil')
left_pupil:set_material(black)
left_pupil:scale(0.1, 0.1, 0.1)
left_pupil:translate(-0.4, 0.6, 1.45)
head:add_child(left_pupil)

local right_pupil = gr.mesh('sphere', 'right_pupil')
right_pupil:set_material(black)
right_pupil:scale(0.1, 0.1, 0.1)
right_pupil:translate(0.4, 0.6, 1.45)
head:add_child(right_pupil)

-- Nose can be represented by a small red sphere
local nose = gr.mesh('sphere', 'nose')
nose:set_material(red)
nose:scale(0.12, 0.12, 0.12)
nose:translate(0.0, 0.25, 1.45)
head:add_child(nose)

-- Create the mouth node
local mouth = gr.mesh('cylinder', 'mouth')
mouth:set_material(red)
-- Scale the cylinder to flatten it into an oval shape appropriate for the mouth
mouth:scale(0.6, 0.2, 0.2) -- The actual values depend on the size of your head model

-- Rotate to align with the face (if necessary)
mouth:rotate('X', 90) 
mouth:translate(0.0, -0.4, 1.1)
head:add_child(mouth)

-- Return the head node
return head