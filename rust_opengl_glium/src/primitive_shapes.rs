use glium::implement_vertex;

#[derive(Copy, Clone)]
pub struct Vertex {
    pub position: (f32, f32, f32),
    pub normal: (f32, f32, f32)
}

impl From<(&(f32, f32, f32), &(f32, f32, f32))> for Vertex {
    fn from((p, n): (&(f32, f32, f32), &(f32, f32, f32))) -> Self {
        Vertex { position: *p, normal: *n }
    }
}

implement_vertex!(Vertex, position, normal);

#[derive(Copy, Clone, Debug)]
pub struct Point {
    pub position: [f32; 3]
}

impl TryFrom<Array1<f32>> for Point {
    type Error = ();

    fn try_from(value: Array1<f32>) -> Result<Self, Self::Error> {
        Ok (Point { position: [value[0], value[1], value[2]] })
    }
}

use std::ops::Sub;
use ndarray::{arr1, Array1};

implement_vertex!(Point, position);

// fn create_square(width: f32, height: f32) -> [Vertex; 4]{
//     let dx = width/2.0;
//     let dy = height/2.0;
//     [ 
//         Vertex { position: [-dx, dy, 0.0], normal: [0.0, 0.0, -1.0], tex_coords: [0.0, 1.0] }, 
//         Vertex { position: [ dx, dy, 0.0], normal: [0.0, 0.0, -1.0], tex_coords: [1.0, 1.0] }, 
//         Vertex { position: [-dx,-dy, 0.0], normal: [0.0, 0.0, -1.0], tex_coords: [0.0, 0.0] }, 
//         Vertex { position: [ dx,-dy, 0.0], normal: [0.0, 0.0, -1.0], tex_coords: [1.0, 0.0] }, 
//     ]
// }
