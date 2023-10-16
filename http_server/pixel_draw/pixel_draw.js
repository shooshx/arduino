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
        this.LINE_WIDTH = 2
        this.PIXEL_SIZE = 20

        this.shadow_canvas = shadow_canvas
        //this.shadow_canvas.style.imageRendering = "pixelated"
        this.shadow_ctx = shadow_canvas.getContext('2d')
        //this.shadow_ctx.imageSmoothingEnabled = false

        this.canvas = canvas
        //this.canvas.style.imageRendering = "pixelated"
        this.ctx = canvas.getContext('2d')

        const l_tool_sz = parseFloat(localStorage['draw_cur_tool_sz'])
        this.tool_radius = (l_tool_sz === undefined) ? 2 : l_tool_sz
        this.tool_color = ColorPicker.parse_hex(localStorage['draw_cur_color'] || '#22AACC')

        this.tool_pradius = this.tool_radius * this.PIXEL_SIZE
        

        this.width = 64
        this.height = 32

        this.shadow_canvas.width = this.width
        this.shadow_canvas.height = this.height

        this.canvas_width = this.width * this.PIXEL_SIZE + this.LINE_WIDTH
        this.canvas_height = this.height * this.PIXEL_SIZE + this.LINE_WIDTH

        this.canvas.width = this.canvas_width
        this.canvas.height = this.canvas_height 

        this.ctx.imageSmoothingEnabled = false

        //this.pixels = new Uint8Array(this.width * this.height * 3)
        this.image_data = this.shadow_ctx.getImageData(0, 0, this.width, this.height)
        this.pixels = this.image_data.data

        //for(let i = 0; i < this.pixels.length; ++i)
        //    this.pixels[i] = 0
        //for(let i = 3; i < this.pixels.length; i += 4)
        //    this.pixels[i] = 255; // all alphas
        
        
    }

    test_pattern()
    {
        for(let i = 0; i < 10; ++i)
            this.setPixel(i, i, 255, 0, 0)
    }

    setPixel(x, y, r, g, b)
    {
        if (x < 0 || y < 0 || x >= this.width || y >= this.height)
            return
        let i = (y * this.width + x)*4
        this.pixels[i] = r
        this.pixels[i+1] = g
        this.pixels[i+2] = b
        this.pixels[i+3] = 255;
    }

    addPixel(x, y, r, g, b, a)
    {
        if (x < 0 || y < 0 || x >= this.width || y >= this.height)
            return        
        let i = (y * this.width + x)*4
        const prev_a = this.pixels[i+3]
        const na = Math.min(prev_a + a, 1.0)
        const nr = (this.pixels[i] *   (1 - a) + r * a)/na
        const ng = (this.pixels[i+1] * (1 - a) + g * a)/na
        const nb = (this.pixels[i+2] * (1 - a) + b * a)/na
        
        this.pixels[i] = nr
        this.pixels[i+1] = ng
        this.pixels[i+2] = nb
        this.pixels[i+3] = na * 255;
    }

    draw_cavnvas()
    {
        this.ctx.fillStyle = '#000000'
        this.ctx.fillRect(0, 0, this.canvas_width, this.canvas_height)
        this.shadow_ctx.putImageData(this.image_data, 0, 0);

        this.ctx.drawImage(this.shadow_canvas, 0, 0, this.canvas_width, this.canvas_height)
        //return

        this.ctx.beginPath();
        for(let x = 1; x < this.canvas_width + 1; x += this.PIXEL_SIZE)
        {
            this.ctx.moveTo(x, 0)
            this.ctx.lineTo(x, this.canvas_height)
        }
        for(let y = 1; y < this.canvas_height + 1; y += this.PIXEL_SIZE)
        {
            this.ctx.moveTo(0, y)
            this.ctx.lineTo(this.canvas_width, y)
        }
        this.ctx.lineWidth = this.LINE_WIDTH
        this.ctx.strokeStyle = '#444444'
        this.ctx.stroke()
    }

    brush(x, y)
    {
        if (this.tool_radius == 0) {
            const ix = Math.trunc(x), iy = Math.trunc(y)
            this.setPixel(ix, iy, this.tool_color.r, this.tool_color.g, this.tool_color.b)
            return
        }

        const x1 = Math.floor(x - this.tool_radius), x2 = Math.ceil(x + this.tool_radius)
        const y1 = Math.floor(y - this.tool_radius), y2 = Math.ceil(y + this.tool_radius)
        for(let ix = x1; ix <= x2; ++ix)
        {
            for(let iy = y1; iy <= y2; ++iy)
            {
                const dx = x - ix-0.5, dy = y - iy-0.5
                const d = Math.min(1.0, this.tool_radius - Math.sqrt(dx*dx + dy*dy))
                if (d < 0)
                    continue
                this.addPixel(ix, iy, this.tool_color.r, this.tool_color.g, this.tool_color.b, d)
                //this.setPixel(ix, iy, this.tool_color.r, this.tool_color.g, this.tool_color.b)
            }
        }

    }

    set_color(c) {
        this.tool_color = { r: c.r, g: c.g, b:c.b }
        localStorage['draw_cur_color'] = c.hex
    }
    set_tool_size(v) {
        this.tool_radius = v
        this.tool_pradius = this.tool_radius * this.PIXEL_SIZE
        localStorage['draw_cur_tool_sz'] = v
    }
    clear() {
        for(let i = 0; i < this.pixels.length; ++i)
            this.pixels[i] = 0
        this.draw_cavnvas()
    }

    tool(x, y, px, py, do_step, act) {
        if (act && do_step)
        {
            this.brush(x, y, do_step)
        }

        this.draw_cavnvas()
    }

    show_tool(px, py) {
        this.ctx.beginPath();
        const r = (this.tool_pradius == 0) ? (0.5 * this.PIXEL_SIZE) : this.tool_pradius
        this.ctx.moveTo(px + r, py)
        this.ctx.arc(px, py, r, 0, 2*Math.PI);
        this.ctx.strokeStyle = '#666666'
        this.ctx.stroke()
    }
}



function connect_events(canvas, s)
{
    let isDown = false, isTouch = false
    let prev_x = null, prev_y = null
    const STEP_SIZE_FACTOR = 0.6
    const call_tool = (e)=>{
        if (e.target.getBoundingClientRect === undefined)
            return
        const rect = e.target.getBoundingClientRect();
        const v_scale = canvas.width / canvas.clientWidth
        const px = (e.clientX - rect.left) * v_scale 
        const py = (e.clientY - rect.top) * v_scale
        const in_canvas = (px >= 0 && py >= 0 && px < s.canvas_width && py < s.canvas_height)
        const x = px / s.PIXEL_SIZE
        const y = py / s.PIXEL_SIZE
        const dx = (x - prev_x), dy = (y - prev_y)
        const d = Math.sqrt(dx*dx + dy*dy)
        const step_pass = (d > STEP_SIZE_FACTOR * s.tool_radius)
        s.tool(x, y, px, py, step_pass, isDown || isTouch);
        if (step_pass)
        {
            prev_x = x
            prev_y = y
        }
        if (in_canvas)
            s.show_tool(px, py)
    }

    canvas.addEventListener('mousedown', (e)=>{
        isDown = true
        prev_x = null
        prev_y = null
        call_tool(e);
    })
    document.addEventListener('mouseup', (e)=>{
        isDown = false;
        call_tool(e);
    })
    canvas.addEventListener('mouseleave', (e)=>{
        call_tool(e);
    })


    canvas.addEventListener('mousemove', (e)=>{
        call_tool(e)
    })

    canvas.addEventListener("touchstart", (e)=>{
        //console.log('t-start')
        e.preventDefault()
        isTouch = true
        prev_x = null
        prev_y = null
        call_tool(e.touches[0]);
    })
    canvas.addEventListener("touchend", (e)=>{
        //console.log('t-end')
        e.preventDefault()
        isTouch = false
        call_tool(e);
    })
    canvas.addEventListener("touchcancel", (e)=>{
        //console.log('t-cancel')
        e.preventDefault()
        isTouch = false
        call_tool(e);
    });
    canvas.addEventListener("touchmove", (e)=>{
        //console.log('t-move')
        e.preventDefault()
        call_tool(e.touches[0])
    });
}

// TODO color picker drag
// TODO color picker dismiss on touch outside
//   local storage

function draw_pixel_onload(root)
{

    const topElem = add_elem(root, 'div', 'draw_top')
    const control_elem = add_elem(topElem, 'div', 'draw_control')

    const canvas = add_elem(topElem, 'canvas', 'draw_canvas')
    canvas.setAttribute('id', 'draw_canvas')
    const shadow_canvas = add_elem(topElem, 'canvas', 'draw_shadow_canvas')
    shadow_canvas.style.visibility = "hidden"

    const s = new DrawState(canvas, shadow_canvas)

    const clear_but = add_elem(control_elem, 'div', 'button')
    clear_but.innerText = 'Clear'
    clear_but.addEventListener('click', (e)=>{
        s.clear();
    })
    const col_in = add_elem(control_elem, 'input', 'draw_col_in')
    ColorEditBox.create_at(col_in, 300, (c)=>{ s.set_color(c) }, {}, ColorPicker.make_hex(s.tool_color, true))
    const size_in = add_elem(control_elem, 'input', ['draw_size_in', 'slider'])
    size_in.setAttribute('type', 'range')
    size_in.setAttribute('min', 0)
    size_in.setAttribute('max', 50)
    size_in.setAttribute('value', s.tool_radius * 10)
    size_in.addEventListener('change', (e)=>{
        s.set_tool_size(parseInt(size_in.value)/10)
    })

    //s.test_pattern()
    s.draw_cavnvas()

    connect_events(canvas, s)
}