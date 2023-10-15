"use strict";

function create_elem(elem_type, cls) {
    let e = document.createElement(elem_type);
    if (cls !== undefined && cls !== null) {
        if (!Array.isArray(cls))
            cls = [cls]
        e.classList = cls.join(" ")
    }
    return e
}
function add_elem(parent, elem_type, cls) {
    let e = create_elem(elem_type, cls)
    parent.appendChild(e)
    return e
}

class DrawState
{
    constructor(canvas, shadow_canvas)
    {
        this.LINE_WIDTH = 1
        this.PIXEL_SIZE = 20

        this.shadow_canvas = shadow_canvas
        //this.shadow_canvas.style.imageRendering = "pixelated"
        this.shadow_ctx = shadow_canvas.getContext('2d')
        //this.shadow_ctx.imageSmoothingEnabled = false

        this.canvas = canvas
        //this.canvas.style.imageRendering = "pixelated"
        this.ctx = canvas.getContext('2d')
        

        this.width = 64
        this.height = 32

        this.shadow_canvas.width = this.width
        this.shadow_canvas.height = this.height

        this.canvas_width = this.width * this.PIXEL_SIZE + this.LINE_WIDTH
        this.canvas_height = this.height * this.PIXEL_SIZE + this.LINE_WIDTH

        this.canvas.width = this.canvas_width
        this.canvas.height = this.canvas_height 

        this.ctx.imageSmoothingEnabled = false
        this.ctx.lineWidth = this.LINE_WIDTH
        this.ctx.strokeStyle = '#444444'

        //this.pixels = new Uint8Array(this.width * this.height * 3)
        this.image_data = this.shadow_ctx.getImageData(0, 0, this.width, this.height)
        this.pixels = this.image_data.data
        for(let i = 3; i < this.pixels.length; i += 4)
            this.pixels[i] = 255; // all alphas
        
        
    }

    test_pattern()
    {
        for(let i = 0; i < 10; ++i)
            this.setPixel(i, i, 255, 0, 0)
    }

    setPixel(x, y, r, g, b)
    {
        let i = (y * this.width + x)*4
        this.pixels[i] = r
        this.pixels[i+1] = g
        this.pixels[i+2] = b
        this.pixels[i+3] = 255;
    }

    draw_cavnvas()
    {
        this.shadow_ctx.putImageData(this.image_data, 0, 0);
        //this.ctx.imageSmoothingEnabled = false
        this.ctx.drawImage(this.shadow_canvas, 0, 0, this.canvas_width, this.canvas_height)
        //return

        for(let x = 0.5; x < this.canvas_width + 1; x += this.PIXEL_SIZE)
        {
            this.ctx.moveTo(x, 0)
            this.ctx.lineTo(x, this.canvas_height)
        }
        for(let y = 0.5; y < this.canvas_height + 1; y += this.PIXEL_SIZE)
        {
            this.ctx.moveTo(0, y)
            this.ctx.lineTo(this.canvas_width, y)
        }
        this.ctx.stroke()
    }

    tool(x, y, px, py, changed_coord) {
        if (changed_coord)
        {
            this.setPixel(x, y, 0, 255, 0)
            this.draw_cavnvas()
        }
    }
}

function connect_events(canvas, s)
{
    let isDown = false
    let prev_x = null, prev_y = null
    const call_tool = (e)=>{
        const rect = e.target.getBoundingClientRect();
        const px = e.clientX - rect.left; 
        const py = e.clientY - rect.top; 
        const x = Math.trunc(px / s.PIXEL_SIZE)
        const y = Math.trunc(py / s.PIXEL_SIZE)
        const changed_coord = x != prev_x || y != prev_y
        s.tool(x, y, px, py, changed_coord);
        prev_x = x
        prev_y = y
    }

    canvas.addEventListener('mousedown', (e)=>{
        isDown = true
        prev_x = null
        prev_y = null
        call_tool(e);
    })
    document.addEventListener('mouseup', (e)=>{
        isDown = false;
    })

    canvas.addEventListener('mousemove', (e)=>{
        if (!isDown)
            return;
        call_tool(e);
    })
}

function draw_pixel_onload(root)
{
    const topElem = add_elem(root, 'div', 'draw_top')
    const controlsElem = add_elem(topElem, 'div', 'draw_control')
    const canvas = add_elem(topElem, 'canvas', 'draw_canvas')
    const shadow_canvas = add_elem(topElem, 'canvas', 'draw_shadow_canvas')
    shadow_canvas.style.visibility = "hidden"

    const s = new DrawState(canvas, shadow_canvas)
    s.test_pattern()
    s.draw_cavnvas()

    connect_events(canvas, s)
}