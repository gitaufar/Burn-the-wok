"""Builds the ranger sprites: walking with the water tank, and spraying with the gun in hand.

Development tool only (needs Python + Pillow); the game itself just loads the PNGs
this script writes. It combines the art team's ranger frames with their sprayer
image, instead of the game drawing the sprayer as a separate picture on top of the body.

    python tools/assets/compose_ranger.py            # writes assets/characters/ranger/*.png
    python tools/assets/compose_ranger.py --preview out.png   # also writes an enlarged sheet

Input  : Burn The Wok/assets/characters/{Front,Back,Left,Right} *.png   (80x80)
         Burn The Wok/assets/items/sprayer_right.png / sprayer_left.png (96x68)
Output : Burn The Wok/assets/characters/ranger/
           {front,back,left,right}_{1..4}.png        walking: tank on the back, arms swing freely
           spray_{front,back,left,right}_{1..4}.png  spraying: gun held in the hands

Spray frames keep the legs of the walk frame but use the upper body of the standing
frame, so the arms hold the gun still while the legs keep walking. Facing the camera
or away from it, the gun is held at the ranger's side pointing down / up.

Every output frame is CANVAS_W x CANVAS_H with the ranger's head centered on
ANCHOR_X and the lowest body pixel on FEET_Y, so the game can draw all frames
with one fixed anchor (see Player::LoadArtwork). The nozzle positions printed at
the end are relative to that anchor; Player.cpp uses them for where water starts.
"""
import argparse
import os

from PIL import Image, ImageDraw

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ASSETS = os.path.join(REPO, "Burn The Wok", "assets")
OUT_DIR = os.path.join(ASSETS, "characters", "ranger")

CANVAS_W, CANVAS_H = 96, 80
ANCHOR_X, FEET_Y = 48, 76

# Source frames, in walk-cycle order. The original filenames are not uniform.
FRAMES = {
    "front": ["Front 1", "Front 2", "Front 3", "Front 4"],
    "back": ["Back 11", "Back 2", "Back 3", "Back 4"],
    "left": ["Left 1", "Left 2", "Left 3", "Left 4"],
    "right": ["Right", "Right 2", "Right 3", "Right 4"],
}

TANK_SCALE = 0.36  # the sprayer art is drawn much larger than the ranger
GUN_SCALE = 0.42
HIP = 33           # rows below the top of the head where the legs start

SKIN = (224, 164, 118, 255)
SKIN_SHADE = (176, 116, 82, 255)
OUTLINE = (38, 28, 30, 255)


def solid(image, threshold=110):
    """Hard alpha edge: resized pixel art must not get a blurry halo."""
    image = image.convert("RGBA")
    r, g, b, a = image.split()
    a = a.point(lambda v: 255 if v >= threshold else 0)
    return Image.merge("RGBA", (r, g, b, a))


def shrink(image, scale):
    size = (max(1, round(image.width * scale)), max(1, round(image.height * scale)))
    return solid(image.resize(size, Image.LANCZOS))


def body_box(image):
    """Bounding box of the ranger, ignoring faint stray pixels in the source art."""
    return solid(image, 128).getbbox()


def head_center_x(image, box):
    """Horizontal center of the head (top quarter of the body)."""
    left, top, right, bottom = box
    head = solid(image, 128).crop((0, top, image.width, top + (bottom - top) // 4)).getbbox()
    return (head[0] + head[2]) / 2.0


def load_parts():
    right = Image.open(os.path.join(ASSETS, "items", "sprayer_right.png")).convert("RGBA")
    left = Image.open(os.path.join(ASSETS, "items", "sprayer_left.png")).convert("RGBA")
    w = right.width
    gun_right = shrink(right.crop((38, 16, 96, 50)), GUN_SCALE)
    return {
        # Tank with its hose loop / the gun with grip and trigger.
        "tank_right": shrink(right.crop((0, 0, 47, 68)), TANK_SCALE),
        "tank_left": shrink(left.crop((w - 47, 0, w, 68)), TANK_SCALE),
        "gun_right": gun_right,
        "gun_left": shrink(left.crop((0, 16, w - 38, 50)), GUN_SCALE),
    }


def aligned(source):
    """The ranger on the shared canvas: head centered on ANCHOR_X, feet on FEET_Y."""
    box = body_box(source)
    left, top, right, bottom = box
    dx = round(ANCHOR_X - head_center_x(source, box))
    dy = FEET_Y - bottom
    body = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    body.alpha_composite(solid(source, 60), (max(0, dx), dy), (max(0, -dx), 0))
    return body, top + dy, bottom - top  # canvas, y of the top of the head, body height


def hand(canvas, x, y):
    """A small fist gripping the gun."""
    d = ImageDraw.Draw(canvas)
    d.rectangle([x - 1, y - 1, x + 2, y + 2], fill=OUTLINE)
    d.rectangle([x, y, x + 1, y + 1], fill=SKIN)
    d.point((x + 1, y + 1), fill=SKIN_SHADE)


def vertical_gun(canvas, x, y_grip, y_nozzle):
    """Spray gun seen from the front/back: a straight barrel from the hand to a red nozzle.

    The side-view gun image cannot be reused here (rotated, its grip sticks out
    sideways), so this one is drawn with the same colors. x = center column.
    """
    d = ImageDraw.Draw(canvas)
    metal, metal_light = (44, 44, 58, 255), (96, 98, 116, 255)
    red, red_dark, red_light = (214, 40, 40, 255), (140, 22, 28, 255), (255, 122, 108, 255)
    yellow = (244, 178, 44, 255)
    step = 1 if y_nozzle > y_grip else -1
    top, bottom = min(y_grip, y_nozzle), max(y_grip, y_nozzle)
    d.rectangle([x - 2, top, x + 2, bottom], fill=OUTLINE)             # outline
    d.rectangle([x - 1, top, x + 1, bottom], fill=metal)               # barrel
    d.line([(x - 1, top), (x - 1, bottom)], fill=metal_light)
    band = y_nozzle - step * 5
    d.rectangle([x - 2, min(band, band + step), x + 2, max(band, band + step)], fill=yellow)
    # Flared nozzle at the far end.
    tip = y_nozzle
    d.rectangle([x - 3, min(tip, tip - step * 3), x + 3, max(tip, tip - step * 3)], fill=red_dark)
    d.rectangle([x - 2, min(tip, tip - step * 3), x + 2, max(tip, tip - step * 3)], fill=red)
    d.point((x - 2, tip - step * 2), fill=red_light)
    d.rectangle([x - 4, tip, x + 4, tip], fill=red_dark)


def with_tank(direction, body, head_top, parts):
    """Walking frame: the tank on the back, nothing in the hands."""
    canvas = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    if direction == "right":
        tank = parts["tank_right"]
        canvas.alpha_composite(tank, (ANCHOR_X - 5 - tank.width, head_top + 17))  # behind the back
        canvas.alpha_composite(body)
    elif direction == "left":
        canvas.alpha_composite(parts["tank_left"], (ANCHOR_X + 5, head_top + 17))
        canvas.alpha_composite(body)
    elif direction == "back":
        tank = parts["tank_right"]
        canvas.alpha_composite(body)
        canvas.alpha_composite(tank, (ANCHOR_X - tank.width // 2 - 1, head_top + 17))  # on the back
    else:  # front: hidden behind the body except for what peeks past the shoulders
        tank = parts["tank_right"]
        canvas.alpha_composite(tank, (ANCHOR_X - tank.width // 2 - 1, head_top + 14))
        canvas.alpha_composite(body)
    return canvas


def spray_body(walk_body, walk_top, idle_body, idle_top):
    """Legs of the walk frame + upper body of the standing frame (arms stay still)."""
    body = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    hip = walk_top + HIP
    body.alpha_composite(walk_body.crop((0, hip, CANVAS_W, CANVAS_H)), (0, hip))
    upper = idle_body.crop((0, 0, CANVAS_W, idle_top + HIP))
    body.alpha_composite(upper, (0, walk_top - idle_top))
    return body


def with_gun(direction, body, head_top, parts):
    """Spraying frame: tank on the back and the gun held in the hands. Returns (canvas, nozzle)."""
    canvas = with_tank(direction, body, head_top, parts)
    if direction == "right":
        gun = parts["gun_right"]
        pos = (ANCHOR_X - 3, head_top + 25)
        canvas.alpha_composite(gun, pos)
        hand(canvas, pos[0] + 12, pos[1] + 7)   # support hand under the barrel
        hand(canvas, pos[0] + 5, pos[1] + 9)    # trigger hand on the grip
        return canvas, (pos[0] + gun.width, pos[1] + round(16 * GUN_SCALE))
    if direction == "left":
        gun = parts["gun_left"]
        pos = (ANCHOR_X + 3 - gun.width, head_top + 25)
        canvas.alpha_composite(gun, pos)
        hand(canvas, pos[0] + gun.width - 14, pos[1] + 7)
        hand(canvas, pos[0] + gun.width - 7, pos[1] + 9)
        return canvas, (pos[0], pos[1] + round(16 * GUN_SCALE))
    if direction == "front":
        # Held in the hanging hand (screen right), pointing at the ground ahead.
        x, grip, tip = ANCHOR_X + 12, head_top + 27, head_top + 43
        vertical_gun(canvas, x, grip, tip)
        hand(canvas, x - 1, grip + 3)
        return canvas, (x, tip)
    # back: same hand (screen left when seen from behind), pointing away from the camera.
    x, grip, tip = ANCHOR_X - 12, head_top + 35, head_top + 19
    vertical_gun(canvas, x, grip, tip)
    hand(canvas, x - 1, grip - 4)
    return canvas, (x, tip)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--preview", help="also write an enlarged contact sheet to this path")
    args = parser.parse_args()

    os.makedirs(OUT_DIR, exist_ok=True)
    parts = load_parts()
    sheet = Image.new("RGBA", (8 * CANVAS_W, 4 * CANVAS_H), (96, 124, 80, 255))
    for row, (direction, names) in enumerate(FRAMES.items()):
        sources = [Image.open(os.path.join(ASSETS, "characters", n + ".png")) for n in names]
        bodies = [aligned(s) for s in sources]
        idle_body, idle_top, _ = bodies[0]
        for col, (body, head_top, height) in enumerate(bodies):
            walk = with_tank(direction, body, head_top, parts)
            walk.save(os.path.join(OUT_DIR, "%s_%d.png" % (direction, col + 1)))
            spray, nozzle = with_gun(direction, spray_body(body, head_top, idle_body, idle_top), head_top, parts)
            spray.save(os.path.join(OUT_DIR, "spray_%s_%d.png" % (direction, col + 1)))
            sheet.alpha_composite(walk, (col * CANVAS_W, row * CANVAS_H))
            sheet.alpha_composite(spray, ((col + 4) * CANVAS_W, row * CANVAS_H))
            if col == 0:
                print("%-5s body height %d px, spray nozzle at (%+d, %+d) from the feet anchor"
                      % (direction, height, nozzle[0] - ANCHOR_X, nozzle[1] - FEET_Y))
    if args.preview:
        sheet.resize((sheet.width * 2, sheet.height * 2), Image.NEAREST).save(args.preview)
    print("wrote 32 frames to", OUT_DIR)


if __name__ == "__main__":
    main()
