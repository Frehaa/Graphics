#![allow(dead_code, unused)]
extern crate glium;
extern crate image; 

use std::f32::consts::PI;
use std::path::Path;
use std::time::{Duration, Instant};

use glutin::event_loop::EventLoop;
use glutin::window::WindowBuilder;
use glutin::ContextBuilder;
use glutin::event::{ Event, WindowEvent, };
use glium::glutin::event_loop;
use glium::{uniform, DrawParameters, Depth, Surface, VertexBuffer, Program, glutin, Display };
use glium::glutin::event::{DeviceEvent, StartCause};
use glium::index::{PrimitiveType, NoIndices};
use glium::texture::RawImage2d;
use image::ImageFormat;

pub mod color;
pub mod world_state;
pub mod primitive_shapes;
pub mod parser;
pub mod math;

use crate::primitive_shapes::{ Point, Vertex };
use crate::parser::{read_obj, read_swp};
use crate::world_state::{WorldState, default_worldstate};

fn load_image_raw<'a, T:AsRef<[u8]>>(cursor: std::io::Cursor<T>, format: ImageFormat) -> RawImage2d<'a, u8> {
    let image = image::load(cursor, format).unwrap().to_rgb8();
    let image_dimensions = image.dimensions();
    RawImage2d::from_raw_rgb_reversed(&image.into_raw(), image_dimensions)
}

fn calculate_perspective(dimensions: (u32, u32)) -> [[f32;4];4] {
    let aspect_ratio = {
        let (width, height) = dimensions;
        height as f32 / width as f32
    };
    let f = 1.0 / (PI / 6.0).tan();
    let (zfar, znear) = (1024.0, 0.1);

    [
        [f * aspect_ratio   , 0.0,              0.0                 ,  0.0],
        [       0.0         ,  f ,              0.0                 ,  0.0],
        [       0.0         , 0.0,  (zfar + znear) / (zfar - znear) ,  1.0],
        [       0.0         , 0.0, -(2.0 * zfar*znear)/(zfar-znear) ,  0.0],
    ]
}

fn main() {
    let event_loop = EventLoop::new();
    let wb = WindowBuilder::new();
    let cb = ContextBuilder::new().with_depth_buffer(24);
    let display = Display::new(wb, cb, &event_loop).unwrap();

    let filename = "resources/torus.obj".to_string();
    let (vertices, indices) = read_obj(&Path::new(&filename), &display);

    let filename = "resources/core.swp".to_string();
    let lines = read_swp(Path::new(&filename));
    let lines : Result<Vec<VertexBuffer<Point>>, _> = lines.iter().map(|x| VertexBuffer::new(&display, x) ).collect();

    if let Err(_) = lines {
        println!("Error reading swap");
        return ;
    }
    let lines = lines.unwrap();

    let vertex_shader = include_str!("shaders/vertex_shader_m1.vert");
    let fragment_shader = include_str!("shaders/fragment_shader_m1.frag");
    let geometry_shader = include_str!("shaders/geometry_shader_wireframe.gs");
    let program = Program::from_source(&display, vertex_shader, fragment_shader, None).unwrap();
    
    let mut wstate = default_worldstate();

    event_loop.run(move |ev, _, control_flow| {
        match ev {
            Event::DeviceEvent { device_id, event } => match event {
                DeviceEvent::MouseMotion { delta } => return wstate.mouse_update(delta),
                DeviceEvent::Key(key) => wstate.key_update(key),
                _ => return
            },
            Event::NewEvents(cause) => match cause {
                StartCause::ResumeTimeReached { start, requested_resume } => (),
                StartCause::Init => (),
                _ => return
            }, 
            Event::MainEventsCleared => return, 
            Event::WindowEvent { event, .. } => match event {
                WindowEvent::CloseRequested => {
                    *control_flow = event_loop::ControlFlow::Exit;
                    return;
                },
                _ => return
            },
            _ => return
        }        

        let next_frame_time = Instant::now() + Duration::from_millis(500);
        *control_flow = event_loop::ControlFlow::WaitUntil(next_frame_time);

        draw_lines(&display, &program, &lines);
    });
}

fn to_vertex_buffer(points: &[Point], display: &Display) -> VertexBuffer<Point> {
    VertexBuffer::new(display, &points).unwrap()
}

fn draw_thing(display: &Display, wstate: &WorldState, vertices: VertexBuffer<Vertex>, program: &Program) {
    let mut target = display.draw();
    let uniforms = uniform! {
        model: wstate.model_transformation,
        view: wstate.view_matrix(),
        perspective: calculate_perspective(target.get_dimensions()),
        u_light: wstate.light_position,
        diffuse_color: wstate.model_color.to_vec3()
    };

    target.clear_color_and_depth((0.0, 0.0, 1.0, 1.0), 1.0);
    target.draw(&vertices, NoIndices(PrimitiveType::TrianglesList), &program, &uniforms, &custom_drawparameters()).unwrap();
    target.finish().unwrap();
}

fn default_drawparameters<'a>() -> DrawParameters<'a> {
    DrawParameters { ..Default::default() }
}

fn custom_drawparameters<'a>() -> DrawParameters<'a> {
    DrawParameters {
        depth: Depth { 
            test: glium::draw_parameters::DepthTest::IfLess, 
            write: true, 
            .. Default::default() 
        },
        blend: glium::draw_parameters::Blend::alpha_blending(),
        .. Default::default()
    }
}
static mut I : usize= 0;

fn draw_lines(display: &Display, program: &Program, lines: &Vec<VertexBuffer<Point>>) {
    let mut target = display.draw();

    let points = vec![Point { position: [0.0, 0.0, 0.0] }, Point { position: [0.5, 0.5, 0.0] }, Point { position: [0.5, 0.0, 0.0] }];
    let buffer = VertexBuffer::new(display, &points).unwrap();

    let scale = [
            [0.3, 0.0, 0.0, 0.0],
            [0.0, 0.3, 0.0, 0.0],
            [0.0, 0.0, 0.3, 0.0],
            [0.0, 0.0, 0.0, 1.0f32],
        ];


    let colors = [ 
        [1.0, 0.0, 0.0, 1.0], 
        [0.0, 1.0, 0.0, 1.0], 
        [0.0, 0.0, 1.0, 1.0], 
        [1.0, 1.0, 1.0, 1.0f32], 
    ];

    unsafe {I = I + 1;}

    target.clear_color_srgb(0.0, 0.0, 0.0, 1.0);
    let param = default_drawparameters();

    for (i, line) in lines.iter().enumerate() {
        unsafe { if i != I % lines.len() { continue } }
        let uniforms = uniform! {
            model: scale, 
            v_c: colors[i % 4]
        };
        target.draw(line, NoIndices(PrimitiveType::LineStrip), &program, &uniforms, &param).unwrap();
    }
    // target.draw(&buffer, NoIndices(PrimitiveType::LineStrip), &program, &uniforms, &param).unwrap();
    target.finish().unwrap();

}