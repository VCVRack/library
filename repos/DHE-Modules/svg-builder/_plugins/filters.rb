require 'color'

module Jekyll
  module Filters
    PX_PER_HP = 15.0
    PX_PER_MM = 2.9527559055
    def mm_to_px(mm)
      mm * PX_PER_MM
    end
    module_function :mm_to_px

    PORT_DIAMETER = mm_to_px(8.4)
    PORT_RADIUS = PORT_DIAMETER / 2.0
    BUTTON_DIAMETER = mm_to_px(6.0)
    BUTTON_RADIUS = BUTTON_DIAMETER / 2.0
    SWITCH_WIDTH = mm_to_px(3.0)
    SWITCH_2_HEIGHT = mm_to_px(6.0)
    SWITCH_3_HEIGHT = mm_to_px(9.0)
    LARGE_KNOB_DIAMETER = mm_to_px(13.0)
    SMALL_KNOB_DIAMETER = mm_to_px(9.0)
    SMALL_LABEL_FONT_SIZE = 7.0 # px
    LARGE_LABEL_FONT_SIZE = 9.0 # px
    PLUGIN_LABEL_FONT_SIZE = 12.0 # px

    class RoundControl
      def initialize(x, y, diameter)
        @x = x
        @y = y
        @diameter = diameter
        @radius = diameter / 2.0
      end

      def top
        @y - @radius
      end

      def left
        @x - @radius
      end

      def right
        @x + @radius
      end

      def width
        @diameter
      end

      def bottom
        @y + @radius
      end
    end

    class Label
      ASCENT_RATIO = 0.66666666666 # For Proxima Nova font

      def initialize(x, y, text, color, size, alignment = :above)
        @x = x
        @y = y
        @text = text.upcase
        @color = color
        @size = size
        @height = @size * ASCENT_RATIO
        @alignment = alignment
        case @alignment
        when :above
          @baseline = "baseline"
          @anchor = "middle"
        when :below
          @baseline = "hanging"
          @anchor = "middle"
        when :right
          @baseline = "middle"
          @anchor = "start"
        end
      end

      def top
        case @alignment
        when :above
          @y - @height
        when :below
          @y
        when :right
          @y - @height / 2.0
        end
      end

      def to_liquid
        %Q[<text dominant-baseline="#{@baseline}" text-anchor="#{@anchor}" fill="#{@color}" x="#{@x}" y="#{@y}" style="font-family:Proxima Nova;font-weight:bold;font-size:#{@size}px">#{@text}</text>]
      end
    end

    class Box
      STROKE_WIDTH = Filters::mm_to_px(0.35)
      PADDING = Filters::mm_to_px(1.0)
      STROKE_INSET = STROKE_WIDTH / 2.0
      CORNER_RADIUS = 3.0

      def initialize(top, right, bottom, left, border, background)
        @top = top - PADDING - STROKE_INSET
        @left = left - PADDING - STROKE_INSET
        @right = right + PADDING + STROKE_INSET
        @bottom = bottom + PADDING + STROKE_INSET
        @width = @right - @left
        @height = @bottom - @top
        @border = border
        @background = background
      end

      def to_liquid
        %Q[
        <rect x="#{@left}" y="#{@top}" width="#{@width}" height="#{@height}"
          stroke-width="#{STROKE_WIDTH}" rx="#{CORNER_RADIUS}" ry="#{CORNER_RADIUS}"
          stroke="#{@border}" fill="#{@background}"/>
        ]
      end
    end

    class PortBox
      BUTTON_EXTRA = BUTTON_DIAMETER + Box::PADDING
      def initialize(x, y, border, background, label_text, label_color, button)
        port = RoundControl.new(x, y, PORT_DIAMETER)
        right = port.right
        left = port.left
        right += BUTTON_EXTRA if button == :right
        left -= BUTTON_EXTRA if button == :left
        @label = Label.new(x, port.top - Box::PADDING, label_text, label_color, SMALL_LABEL_FONT_SIZE)
        @box = Box.new(@label.top, right, port.bottom, left, border, background)
      end

      def to_liquid
        @box.to_liquid + @label.to_liquid
      end
    end

    class RoundControlLabel
      def initialize(x, y, diameter, text, color, font_size)
        control = RoundControl.new(x, y, diameter)
        @label = Label.new(x, control.top - Box::PADDING, text, color, font_size)
      end

      def to_liquid
        @label.to_liquid
      end
    end

    class RectangularControlLabels
      def initialize(x, y, width, height, color, top_label, bottom_label, right_label)
        top = y - height / 2.0
        bottom = y + height / 2.0
        left = x - width / 2.0
        right = x + width / 2.0
        @labels = [
          Label.new(x, top - Box::PADDING - Box::STROKE_INSET, top_label, color, SMALL_LABEL_FONT_SIZE, :above),
          Label.new(x, bottom + Box::PADDING + Box::STROKE_INSET, bottom_label, color, SMALL_LABEL_FONT_SIZE, :below)
        ]
        @labels << Label.new(right + Box::PADDING / 2.0 + Box::STROKE_INSET, y, right_label, color, SMALL_LABEL_FONT_SIZE, :right) if right_label
      end

      def to_liquid
        @labels.map(&:to_liquid).join
      end
    end

    class Panel
      HEIGHT = Filters::mm_to_px(128.7)
      PLUGIN_LABEL_INSET = Filters::mm_to_px(9.0)

      def initialize(width, name, color, background)
        @width = width
        @color = color
        @background = background
        center_x = width / 2.0
        @plugin_name = Label.new(center_x, PLUGIN_LABEL_INSET, name, color, PLUGIN_LABEL_FONT_SIZE, :above)
        @plugin_author = Label.new(center_x, HEIGHT - PLUGIN_LABEL_INSET, "DHE", color, PLUGIN_LABEL_FONT_SIZE, :below)
      end

      def background_svg
        %Q[<rect x="0" y="0" width="#{@width}" height="#{HEIGHT}" stroke="#{@color}" fill="#{@background}" stroke-width="1"/>]
      end

      def to_liquid
        background_svg + @plugin_name.to_liquid + @plugin_author.to_liquid
      end
    end


    def labeled_round_control(x_mm, y_mm, diameter, label, color, font_size)
      x_px = Filters::mm_to_px(x_mm)
      y_px = Filters::mm_to_px(y_mm)
      RoundControlLabel.new(x_px, y_px, diameter, label, color, font_size)
    end

    def port_box(x_mm, y_mm, border, background, label, label_color, button)
      x_px = Filters::mm_to_px(x_mm)
      y_px = Filters::mm_to_px(y_mm)
      PortBox.new(x_px, y_px, border, background, label, label_color, button)
    end

    def button(page, x_mm, y_mm, label)
      labeled_round_control(x_mm, y_mm, BUTTON_DIAMETER, label, dark(page), SMALL_LABEL_FONT_SIZE)
    end

    def cv(page, x_mm, y_mm)
      port_box(x_mm, y_mm, "none", "none", "CV", dark(page), :none)
    end

    def in_port_button(page, x_mm, y_mm, label)
      port_box(x_mm, y_mm, dark(page), light(page), label, dark(page), :right)
    end

    def out_port_button(page, x_mm, y_mm, label)
      port_box(x_mm, y_mm, dark(page), dark(page), label, light(page), :left)
    end

    def in_port(page, x_mm, y_mm, label)
      port_box(x_mm, y_mm, dark(page), light(page), label, dark(page), :none)
    end

    def out_port(page, x_mm, y_mm, label)
      port_box(x_mm, y_mm, dark(page), dark(page), label, light(page), :none)
    end

    def large_knob(page, x_mm, y_mm, label)
      labeled_round_control(x_mm, y_mm, LARGE_KNOB_DIAMETER, label, dark(page), LARGE_LABEL_FONT_SIZE)
    end

    def small_knob(page, x_mm, y_mm, label)
      labeled_round_control(x_mm, y_mm, SMALL_KNOB_DIAMETER, label, dark(page), SMALL_LABEL_FONT_SIZE)
    end

    def hslcolor(page)
      Color::HSL.new(*page['color'])
    end

    def rgbhex(color)
      "##{color.to_rgb.hex}"
    end

    def light(page)
      hsl = hslcolor(page)
      hsl.l = 0.97
      rgbhex(hsl)
    end

    def dark(page)
      return rgbhex(hslcolor(page))
    end

    def shape_switch(page, x_mm, y_mm)
      switch(page, x_mm, y_mm, 'S', 'J')
    end

    def switch(page, x_mm, y_mm, top_label, bottom_label, right_label = nil)
      height = right_label ? SWITCH_3_HEIGHT : SWITCH_2_HEIGHT
      x_px = Filters::mm_to_px(x_mm)
      y_px = Filters::mm_to_px(y_mm)
      RectangularControlLabels.new(x_px, y_px, SWITCH_WIDTH, height, dark(page), top_label, bottom_label, right_label)
    end

    def panel(page, width_hp)
      width_px = width_hp * PX_PER_HP
      Panel.new(width_px, page['title'], dark(page), light(page))
    end

    def connector(page, x1_mm, y1_mm, x2_mm, y2_mm)
      x1 = Filters::mm_to_px(x1_mm)
      y1 = Filters::mm_to_px(y1_mm)
      x2 = Filters::mm_to_px(x2_mm)
      y2 = Filters::mm_to_px(y2_mm)
      %Q[<line x1="#{x1}" y1="#{y1}" x2="#{x2}" y2="#{y2}" stroke="#{dark(page)}" stroke-width="#{Box::STROKE_WIDTH}" />]
    end
  end
end

Liquid::Template.register_filter(Jekyll::Filters)
