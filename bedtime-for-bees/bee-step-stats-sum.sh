
echo_img_html_f()
{
  echo "<img src='$1' width='400'/>"
}

bee_id_field="bee_id_i"
pos_delta_field="pos_delta_f"

for yyyymmdd in 2015-09-07 2015-09-08 2015-09-09
do

  # all bees (percentiles)
  for suffix in p10to40 p50 p50to70 p80to90
  do
    case $suffix in
      p10to40)
        multi_bee_percentiles="10 20 30 40"
        y_axis=250
        ;;
      p50)
        multi_bee_percentiles="50"
        y_axis=500
        ;;
      p50to70)
        multi_bee_percentiles="50 60 70"
        y_axis=3000
        ;;
      p80to90)
        multi_bee_percentiles="80 90"
        y_axis=150000
        ;;
      *)
        continue
        ;;
    esac

    case $suffix in
      p50)
        plot_fmt="tab:orange"
        ;;
      *)
        plot_fmt=".-"
        ;;
    esac

    q="${bee_id_field}:[*+TO+*]"

    png_file="./bee-all-${suffix}-step-stats-sum-${yyyymmdd}.png"
    echo_img_html_f $png_file

    ./bee-step-stats.py ${png_file} "${q}" \
    "${pos_delta_field}:[0+TO+*]" \
    --multi-bee-percentile ${multi_bee_percentiles} \
    --timestamp-range-start "${yyyymmdd}T00:00:00.000Z" \
    --timestamp-range-end   "${yyyymmdd}T23:59:59.999Z" \
    --plot-fmt "${plot_fmt}" \
    --plot-axis -1 25 -1 ${y_axis} \
    --plot-title "${yyyymmdd} percentile((${multi_bee_percentiles}),sum(${pos_delta_field}))
${q}"
  done

  # one, several or all bee(s)
  for who in all 104 like104 unlike104
  do

    case $who in
      all)
        q="${bee_id_field}:[*+TO+*]"
        y_axis=75000000
        ;;
      like104)
        if [[ -z "${Q_LIKE_BEE_104}" ]]
        then
          continue
        fi
        q="${bee_id_field}:(${Q_LIKE_BEE_104})"
        y_axis=1500000
        ;;
      unlike104)
        if [[ -z "${Q_UNLIKE_BEE_104}" ]]
        then
          continue
        fi
        q="${bee_id_field}:(${Q_UNLIKE_BEE_104})"
        y_axis=1500000
        ;;
      *)
        q="${bee_id_field}:${who}"
        y_axis=150000
        ;;
    esac

    png_file="./bee-${who}-step-stats-sum-${yyyymmdd}.png"
    echo_img_html_f $png_file

    ./bee-step-stats.py ${png_file} "${q}" \
    "${pos_delta_field}:[0+TO+*]" \
    --timestamp-range-start "${yyyymmdd}T00:00:00.000Z" \
    --timestamp-range-end   "${yyyymmdd}T23:59:59.999Z" \
    --plot-axis -1 25 -1 ${y_axis} \
    --plot-title "${yyyymmdd} sum(${pos_delta_field})
${q}"
  done

done

