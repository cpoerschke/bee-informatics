#!/usr/bin/env python

import argparse
import qrcode

if __name__ == "__main__":

  parser = argparse.ArgumentParser()

  parser.add_argument("output_file_name", type=str)
  parser.add_argument("data", type=str)
  parser.add_argument("--version", type=int, default=None)
  parser.add_argument("--error_correction", type=int, default=qrcode.constants.ERROR_CORRECT_M)
  parser.add_argument("--box_size", type=int, default=10)
  parser.add_argument("--border", type=int, default=4)
  parser.add_argument("--fit", type=bool, default=True)
  parser.add_argument("--fill_color", type=str, default="black")
  parser.add_argument("--back_color", type=str, default="white")

  args = parser.parse_args()

  qr = qrcode.QRCode(
    version=args.version,
    error_correction=args.error_correction,
    box_size=args.box_size,
    border=args.border
  )
  qr.add_data(args.data)
  qr.make(fit=args.fit)

  img = qr.make_image(fill_color=args.fill_color, back_color=args.back_color)
  img.save(args.output_file_name)

