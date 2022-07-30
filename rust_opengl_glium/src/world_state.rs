use glium::glutin::event::{ KeyboardInput, ElementState, VirtualKeyCode };
use std::f32::consts::PI;

use crate::color::Color;
use crate::math::{ add_mut, rotate };

#[derive(Debug)]
pub struct WorldState {
    pub light_position: [f32; 3],
    pub camera_position: [f32; 3],
    pub camera_direction: [f32; 3],
    pub camera_orientation: [f32; 3],
    pub model_transformation: [[f32; 4]; 4],
    pub model_color: Color,
    pub pressed: bool
}

impl WorldState {
    pub fn view_matrix(&self) -> [[f32; 4]; 4] {
        let position = self.camera_position;
        let direction = self.camera_direction;
        let up = self.camera_orientation;
        let f = {
            let f = direction;
            let len = f[0] * f[0] + f[1] * f[1] + f[2] * f[2];
            f.map(|x| x/len.sqrt())
        };
        let s_norm = {
            let s = [up[1] * f[2] - up[2] * f[1],
                    up[2] * f[0] - up[0] * f[2],
                    up[0] * f[1] - up[1] * f[0]];
            let len = s[0] * s[0] + s[1] * s[1] + s[2] * s[2];
            s.map(|x| x/len.sqrt())
        };
        let u = [f[1] * s_norm[2] - f[2] * s_norm[1],
                f[2] * s_norm[0] - f[0] * s_norm[2],
                f[0] * s_norm[1] - f[1] * s_norm[0]];
        let p = [-position[0] * s_norm[0] - position[1] * s_norm[1] - position[2] * s_norm[2],
                -position[0] * u[0] - position[1] * u[1] - position[2] * u[2],
                -position[0] * f[0] - position[1] * f[1] - position[2] * f[2]];
        [
            [s_norm[0], u[0], f[0], 0.0],
            [s_norm[1], u[1], f[1], 0.0],
            [s_norm[2], u[2], f[2], 0.0],
            [p[0], p[1], p[2], 1.0],
        ]
    }

    pub fn key_update(&mut self, key: KeyboardInput) -> () {
        let light_move = 0.1;
        match key {
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::C), state: ElementState::Pressed, ..} => {
                self.model_color = self.model_color.next_color();
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::Left), state: ElementState::Pressed, ..} => {
                self.light_position[0] -= light_move;
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::Right), state: ElementState::Pressed, ..} => {
                self.light_position[0] += light_move;
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::Up), state: ElementState::Pressed, ..} => {
                self.light_position[1] += light_move;
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::Down), state: ElementState::Pressed, ..} => {
                self.light_position[1] -= light_move;
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::W), state: ElementState::Pressed, ..} => {
                add_mut(&mut self.camera_position, &self.camera_direction.map(|x| x * 0.1));
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::S), state: ElementState::Pressed, ..} => {
                add_mut(&mut self.camera_position, &self.camera_direction.map(|x| x * -0.1));
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::D), state: ElementState::Pressed, ..} => {
                let mut direction = self.camera_direction;
                let (x, z) = rotate((direction[0], direction[2]), PI/2.0);
                direction[0] = x;
                direction[2] = z;
                add_mut(&mut self.camera_position, &direction.map(|x| x * -0.1));
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::A), state: ElementState::Pressed, ..} => {
                let mut direction = self.camera_direction;
                let (x, z) = rotate((direction[0], direction[2]), -PI/2.0);
                direction[0] = x;
                direction[2] = z;
                add_mut(&mut self.camera_position, &direction.map(|x| x * -0.1));
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F1), state: ElementState::Pressed, ..} => {
                println!("Position = {:?}", self.camera_position);
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F2), state: ElementState::Pressed, ..} => {
                println!("Direction ={:?}", self.camera_direction);
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F3), state: ElementState::Pressed, ..} => {
                println!("Orientation = {:?}", self.camera_orientation);
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F4), state: ElementState::Pressed, ..} => {
                println!("Light = {:?}", self.light_position);
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F5), state: ElementState::Pressed, ..} => {
                println!("Transform = {:?}", self.model_transformation);
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F6), state: ElementState::Pressed, ..} => {
                println!("Color = {:?}", self.model_color);
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F7), state: ElementState::Pressed, ..} => {
                println!("Pressed = {:#?}", self.pressed);
            },
            KeyboardInput { virtual_keycode: Some(VirtualKeyCode::F8), state: ElementState::Pressed, ..} => {
                println!("{:#?}", self);
            },
            _ => return
        }
    }

    pub fn mouse_update(&mut self, delta: (f64, f64)) {
        if self.pressed {
            let x = self.camera_direction[0];
            let i = self.camera_direction[2];
            let (x, y) = rotate((x,i), -0.03 * delta.0 as f32);
            self.camera_direction[0] = x;
            self.camera_direction[2] = y;

            // let x = self.camera_orientation[1];
            // let y = self.camera_orientation[2];
            // let (x, y) = rotate((x,i), 0.03 * delta.1 as f32);
            // self.camera_orientation[1] = x;
            // self.camera_orientation[2] = y;
        }
    }

    pub fn set_pressed(&mut self, b: bool) {
        self.pressed = b;
    }


}

pub fn default_worldstate() -> WorldState {
    WorldState {
        light_position: [0.0, 0.0, 1.0f32],
        model_color: Color::RED,
        model_transformation: [
            [1.0, 0.0, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 1.0, 1.0f32],
        ],
        camera_position: [0.0, 0.0, -3.0],
        camera_direction: [0.0, 0.0, 1.0],
        camera_orientation: [0.0, 1.0, 0.0],
        pressed: false
    }
}