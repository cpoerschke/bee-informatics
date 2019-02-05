
echo_img_html_f()
{
  echo "<img src='$1' width='400'/>"
}

bee_id_field="bee_id_i"
pos_delta_field="pos_delta_f"

percentiles="25 75"

for yyyymmdd in 2015-09-07 2015-09-08 2015-09-09
do

  for bee_id_i in 104
  do
    q="${bee_id_field}:${bee_id_i}"

    for unit in px mm
    do

      case $unit in
        px)
          y_axis=50
          plot_y_multiplier=
          ;;
        mm)
          y_axis=9
          plot_y_multiplier=0.18
          ;;
        *)
          continue
          ;;
      esac

      if [[ -z "${plot_y_multiplier}" ]]
      then
        plot_y_multiplier_opt=
        plot_y_multiplier_desc=
      else
        plot_y_multiplier_opt="--plot-y-multiplier ${plot_y_multiplier}"
        plot_y_multiplier_desc=" x ${plot_y_multiplier}"
      fi

      png_file="./bee-${bee_id_i}-step-stats-percentile-${yyyymmdd}-${unit}.png"
      echo_img_html_f $png_file

      ./bee-step-stats.py ${png_file} "${q}" \
      "${pos_delta_field}:[0+TO+*]" \
      --timestamp-range-start "${yyyymmdd}T00:00:00.000Z" \
      --timestamp-range-end   "${yyyymmdd}T23:59:59.999Z" \
      --pos-delta-percentile ${percentiles} \
      ${plot_y_multiplier_opt} \
      --plot-axis -1 25 0 ${y_axis} \
      --plot-title "${yyyymmdd} percentile((${percentiles}),${pos_delta_field})${plot_y_multiplier_desc}
${q}"
    done

  done

done

