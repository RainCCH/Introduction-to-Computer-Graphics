
rootnode = gr.node('root')
rootnode:translate(0.0, 0.15, -3.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('sphere', 'torso')
torso:scale(0.25, 0.5, 0.15)
torso:set_material(red)
rootnode:add_child(torso)

shoulder_joint = gr.joint('shoulder_joint', {-90, 0, 90}, {-90, 0, 90})
shoulder_joint:translate(0.0, 0.5, 0.0)
rootnode:add_child(shoulder_joint)

shoulders = gr.mesh('sphere', 'shoulders')
shoulders:scale(0.4, 0.08, 0.15)
shoulders:set_material(white)
shoulder_joint:add_child(shoulders)

neck_joint = gr.joint('neck_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
neck_joint:translate(0.0, 0.2, 0.0)
shoulder_joint:add_child(neck_joint)

neck = gr.mesh('sphere', 'neck')
neck:scale(0.1, 0.15, 0.1)
neck:set_material(white)
neck_joint:add_child(neck)

head_joint = gr.joint('head_joint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
head_joint:translate(0.0, 0.1, 0.0)
neck_joint:add_child(head_joint)

head = gr.mesh('sphere', 'head')
head:scale(0.2, 0.2, 0.2)
head:set_material(blue)
head_joint:add_child(head)

left_eye = gr.mesh('sphere', 'left_eye')
left_eye:set_material(white)
left_eye:scale(0.5, 0.5, 0.5) -- Scale to make the eye smaller
left_eye:translate(-0.4, 0.45, 1.0) -- Position the eye on the face
head:add_child(left_eye)

-- Right eye, similar to the left eye
right_eye = gr.mesh('sphere', 'right_eye')
right_eye:set_material(white)
right_eye:scale(0.5, 0.5, 0.5)
right_eye:translate(0.4, 0.45, 1.0)
head:add_child(right_eye)

-- Pupils can be created using smaller, black spheres or as part of the eye texture if textures are supported
left_pupil = gr.mesh('sphere', 'left_pupil')
left_pupil:set_material(black)
left_pupil:scale(0.1, 0.1, 0.1)
left_pupil:translate(-0.4, 0.45, 1.5)
head:add_child(left_pupil)

right_pupil = gr.mesh('sphere', 'right_pupil')
right_pupil:set_material(black)
right_pupil:scale(0.1, 0.1, 0.1)
right_pupil:translate(0.4, 0.45, 1.5)
head:add_child(right_pupil)

-- Nose can be represented by a small red sphere
nose = gr.mesh('sphere', 'nose')
nose:set_material(red)
nose:scale(0.25, 0.25, 0.25)
nose:translate(0.0, 0.1, 1.0)
head:add_child(nose)

left_upper_arm_joint = gr.joint('left_upper_arm_joint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
left_upper_arm_joint:translate(0.375, -0.2, 0.0)
shoulder_joint:add_child(left_upper_arm_joint)

left_upper_arm = gr.mesh('sphere', 'left_upper_arm')
left_upper_arm:scale(0.1, 0.2, 0.1)
left_upper_arm:set_material(green)
left_upper_arm_joint:add_child(left_upper_arm)

left_lower_arm_joint = gr.joint('left_lower_arm_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
left_lower_arm_joint:translate(0.0, -0.325, 0.0)
left_upper_arm_joint:add_child(left_lower_arm_joint)

left_lower_arm = gr.mesh('sphere', 'left_lower_arm')
left_lower_arm:scale(0.08, 0.25, 0.1)
left_lower_arm:set_material(red)
left_lower_arm_joint:add_child(left_lower_arm)

left_hand_joint = gr.joint('hand_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
left_hand_joint:translate(0.0, -0.325, 0.0)
left_lower_arm_joint:add_child(left_hand_joint)

left_hand = gr.mesh('sphere', 'hand')
left_hand:scale(0.08, 0.08, 0.08)
left_hand:set_material(white)
left_hand_joint:add_child(left_hand)

right_upper_arm_joint = gr.joint('right_upper_arm_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
right_upper_arm_joint:translate(-0.375, -0.2, 0.0)
shoulder_joint:add_child(right_upper_arm_joint)

right_upper_arm = gr.mesh('sphere', 'right_upper_arm')
right_upper_arm:scale(0.1, 0.2, 0.1)
right_upper_arm:set_material(green)
right_upper_arm_joint:add_child(right_upper_arm)

right_lower_arm_joint = gr.joint('right_lower_arm_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
right_lower_arm_joint:translate(0.0, -0.325, 0.0)
right_upper_arm_joint:add_child(right_lower_arm_joint)

right_lower_arm = gr.mesh('sphere', 'right_lower_arm')
right_lower_arm:scale(0.08, 0.25, 0.1)
right_lower_arm:set_material(red)
right_lower_arm_joint:add_child(right_lower_arm)

right_hand_joint = gr.joint('right_hand_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
right_hand_joint:translate(0.0, -0.325, 0.0)
right_lower_arm_joint:add_child(right_hand_joint)

right_hand = gr.mesh('sphere', 'right_hand')
right_hand:scale(0.08, 0.08, 0.08)
right_hand:set_material(white)
right_hand_joint:add_child(right_hand)

hip_joint = gr.joint('hip_joint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
hip_joint:translate(0.0, -0.5, 0.0)
rootnode:add_child(hip_joint)

hips = gr.mesh('sphere', 'hips')
hips:scale(0.25, 0.1, 0.15)
hips:set_material(white)
hip_joint:add_child(hips)

left_upper_leg_joint = gr.joint('left_upper_leg_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
left_upper_leg_joint:translate(0.15, -0.3, 0.0)
hip_joint:add_child(left_upper_leg_joint)

left_upper_leg = gr.mesh('sphere', 'left_upper_leg')
left_upper_leg:scale(0.15, 0.3, 0.15)
left_upper_leg:set_material(green)
left_upper_leg_joint:add_child(left_upper_leg)

left_lower_leg_joint = gr.joint('left_lower_leg_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
left_lower_leg_joint:translate(0.0, -0.4, 0.0)
left_upper_leg_joint:add_child(left_lower_leg_joint)

left_lower_leg = gr.mesh('sphere', 'left_lower_leg')
left_lower_leg:scale(0.12, 0.35, 0.12)
left_lower_leg:set_material(red)
left_lower_leg_joint:add_child(left_lower_leg)

left_foot_joint = gr.joint('left_foot_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
left_foot_joint:translate(0.0, -0.4, 0.1)
left_lower_leg_joint:add_child(left_foot_joint)

left_foot = gr.mesh('sphere', 'left_foot')
left_foot:scale(0.12, 0.08, 0.2)
left_foot:set_material(white)
left_foot_joint:add_child(left_foot)

right_upper_leg_joint = gr.joint('right_upper_leg_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
right_upper_leg_joint:translate(-0.15, -0.3, 0.0)
hip_joint:add_child(right_upper_leg_joint)

right_upper_leg = gr.mesh('sphere', 'right_upper_leg')
right_upper_leg:scale(0.15, 0.3, 0.15)
right_upper_leg:set_material(green)
right_upper_leg_joint:add_child(right_upper_leg)

right_lower_leg_joint = gr.joint('right_lower_leg_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
right_lower_leg_joint:translate(0.0, -0.4, 0.0)
right_upper_leg_joint:add_child(right_lower_leg_joint)

right_lower_leg = gr.mesh('sphere', 'right_lower_leg')
right_lower_leg:scale(0.12, 0.35, 0.12)
right_lower_leg:set_material(red)
right_lower_leg_joint:add_child(right_lower_leg)

right_foot_joint = gr.joint('right_foot_joint', {-45.0, 0.0, 45.0}, {-45.0, 0.0, 45.0})
right_foot_joint:translate(0.0, -0.4, 0.1)
right_lower_leg_joint:add_child(right_foot_joint)

right_foot = gr.mesh('sphere', 'right_foot')
right_foot:scale(0.12, 0.08, 0.2)
right_foot:set_material(white)
right_foot_joint:add_child(right_foot)

return rootnode
