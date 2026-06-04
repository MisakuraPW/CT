#!/usr/bin/env python3
"""Generate report-ready SVG figures from exported dataset CSV files.

The script intentionally uses only the Python standard library so it can run on
classroom machines without installing matplotlib or pandas.
"""

from __future__ import annotations

import argparse
import csv
from collections import Counter, defaultdict
from pathlib import Path
from statistics import mean


SVG_STYLE = """
<style>
text { font-family: "Microsoft YaHei", "Segoe UI", Arial, sans-serif; fill: #202124; }
.title { font-size: 20px; font-weight: 700; }
.label { font-size: 12px; }
.tick { font-size: 11px; fill: #5f6368; }
.grid { stroke: #e0e0e0; stroke-width: 1; }
.axis { stroke: #444; stroke-width: 1.2; }
</style>
""".strip()


def read_csv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        raise FileNotFoundError(f"CSV not found: {path}")

    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def to_float(value: str, default: float = 0.0) -> float:
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def to_int(value: str, default: int = 0) -> int:
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return default


def xml_escape(value: object) -> str:
    text = str(value)
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def svg_root(width: int, height: int, body: str) -> str:
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
        f'viewBox="0 0 {width} {height}">\n{SVG_STYLE}\n{body}\n</svg>\n'
    )


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def bar_chart(
    title: str,
    labels: list[str],
    values: list[float],
    output_path: Path,
    *,
    color: str = "#2e7d32",
    value_format: str = "{:.3f}",
) -> None:
    width = 900
    height = 520
    left = 90
    right = 40
    top = 70
    bottom = 100
    chart_width = width - left - right
    chart_height = height - top - bottom

    max_value = max(values, default=0.0)
    max_value = 1.0 if max_value <= 0 else max_value * 1.12
    bar_gap = 18
    bar_width = max(22, (chart_width - bar_gap * max(0, len(values) - 1)) / max(1, len(values)))

    parts = [
        f'<text class="title" x="{left}" y="36">{xml_escape(title)}</text>',
        f'<line class="axis" x1="{left}" y1="{top + chart_height}" x2="{left + chart_width}" y2="{top + chart_height}" />',
        f'<line class="axis" x1="{left}" y1="{top}" x2="{left}" y2="{top + chart_height}" />',
    ]

    for step in range(5):
        value = max_value * step / 4
        y = top + chart_height - chart_height * step / 4
        parts.append(f'<line class="grid" x1="{left}" y1="{y:.1f}" x2="{left + chart_width}" y2="{y:.1f}" />')
        parts.append(f'<text class="tick" x="{left - 12}" y="{y + 4:.1f}" text-anchor="end">{value_format.format(value)}</text>')

    for index, (label, value) in enumerate(zip(labels, values)):
        x = left + index * (bar_width + bar_gap)
        bar_height = chart_height * value / max_value if max_value else 0
        y = top + chart_height - bar_height
        parts.append(f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_width:.1f}" height="{bar_height:.1f}" fill="{color}" rx="2" />')
        parts.append(f'<text class="label" x="{x + bar_width / 2:.1f}" y="{y - 8:.1f}" text-anchor="middle">{value_format.format(value)}</text>')
        parts.append(
            f'<text class="tick" x="{x + bar_width / 2:.1f}" y="{top + chart_height + 22}" '
            f'text-anchor="middle" transform="rotate(25 {x + bar_width / 2:.1f} {top + chart_height + 22})">{xml_escape(label)}</text>'
        )

    write_text(output_path, svg_root(width, height, "\n".join(parts)))


def grouped_status_chart(rows: list[dict[str, str]], output_path: Path) -> None:
    datasets = sorted({row.get("dataset_kind", "unknown") or "unknown" for row in rows})
    ok_counts = []
    failed_counts = []
    for dataset in datasets:
        status = Counter(row.get("status", "unknown") for row in rows if row.get("dataset_kind") == dataset)
        ok_counts.append(status.get("ok", 0))
        failed_counts.append(status.get("failed", 0))

    width = 900
    height = 520
    left = 90
    top = 70
    chart_width = 760
    chart_height = 330
    max_value = max(ok_counts + failed_counts + [1])
    group_width = chart_width / max(1, len(datasets))
    bar_width = min(70, group_width * 0.32)

    parts = [
        f'<text class="title" x="{left}" y="36">Dataset Case Status</text>',
        f'<line class="axis" x1="{left}" y1="{top + chart_height}" x2="{left + chart_width}" y2="{top + chart_height}" />',
        f'<line class="axis" x1="{left}" y1="{top}" x2="{left}" y2="{top + chart_height}" />',
        f'<rect x="{left + 560}" y="22" width="14" height="14" fill="#2e7d32" /><text class="label" x="{left + 580}" y="34">ok</text>',
        f'<rect x="{left + 620}" y="22" width="14" height="14" fill="#c62828" /><text class="label" x="{left + 640}" y="34">failed</text>',
    ]

    for step in range(5):
        value = max_value * step / 4
        y = top + chart_height - chart_height * step / 4
        parts.append(f'<line class="grid" x1="{left}" y1="{y:.1f}" x2="{left + chart_width}" y2="{y:.1f}" />')
        parts.append(f'<text class="tick" x="{left - 12}" y="{y + 4:.1f}" text-anchor="end">{value:.0f}</text>')

    for index, dataset in enumerate(datasets):
        group_x = left + index * group_width + group_width / 2
        for offset, value, color in [(-bar_width * 0.6, ok_counts[index], "#2e7d32"), (bar_width * 0.6, failed_counts[index], "#c62828")]:
            bar_height = chart_height * value / max_value
            x = group_x + offset - bar_width / 2
            y = top + chart_height - bar_height
            parts.append(f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_width:.1f}" height="{bar_height:.1f}" fill="{color}" rx="2" />')
            parts.append(f'<text class="label" x="{x + bar_width / 2:.1f}" y="{y - 8:.1f}" text-anchor="middle">{value}</text>')
        parts.append(f'<text class="tick" x="{group_x:.1f}" y="{top + chart_height + 25}" text-anchor="middle">{xml_escape(dataset)}</text>')

    write_text(output_path, svg_root(width, height, "\n".join(parts)))


def line_chart(title: str, series: dict[str, list[tuple[int, float]]], output_path: Path) -> None:
    width = 980
    height = 560
    left = 80
    right = 180
    top = 70
    bottom = 70
    chart_width = width - left - right
    chart_height = height - top - bottom
    colors = ["#c62828", "#1565c0", "#2e7d32", "#6a1b9a", "#ef6c00"]

    max_x = max((point[0] for points in series.values() for point in points), default=1)
    max_y = max((point[1] for points in series.values() for point in points), default=0.0)
    max_y = 1.0 if max_y <= 0 else min(1.0, max_y * 1.2)

    parts = [
        f'<text class="title" x="{left}" y="36">{xml_escape(title)}</text>',
        f'<line class="axis" x1="{left}" y1="{top + chart_height}" x2="{left + chart_width}" y2="{top + chart_height}" />',
        f'<line class="axis" x1="{left}" y1="{top}" x2="{left}" y2="{top + chart_height}" />',
    ]

    for step in range(5):
        y_value = max_y * step / 4
        y = top + chart_height - chart_height * step / 4
        parts.append(f'<line class="grid" x1="{left}" y1="{y:.1f}" x2="{left + chart_width}" y2="{y:.1f}" />')
        parts.append(f'<text class="tick" x="{left - 12}" y="{y + 4:.1f}" text-anchor="end">{y_value:.3f}</text>')

    for step in range(6):
        x_value = max_x * step / 5
        x = left + chart_width * step / 5
        parts.append(f'<text class="tick" x="{x:.1f}" y="{top + chart_height + 24}" text-anchor="middle">{x_value:.0f}</text>')

    for index, (name, points) in enumerate(series.items()):
        color = colors[index % len(colors)]
        if not points:
            continue
        path_points = []
        for x_value, y_value in points:
            x = left + chart_width * (x_value / max_x if max_x else 0)
            y = top + chart_height - chart_height * (y_value / max_y if max_y else 0)
            path_points.append(f"{x:.1f},{y:.1f}")
        parts.append(f'<polyline points="{" ".join(path_points)}" fill="none" stroke="{color}" stroke-width="2.4" />')
        for x_value, y_value in points:
            x = left + chart_width * (x_value / max_x if max_x else 0)
            y = top + chart_height - chart_height * (y_value / max_y if max_y else 0)
            parts.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.5" fill="{color}" />')
        legend_y = top + 20 + index * 24
        parts.append(f'<line x1="{left + chart_width + 28}" y1="{legend_y}" x2="{left + chart_width + 52}" y2="{legend_y}" stroke="{color}" stroke-width="3" />')
        parts.append(f'<text class="label" x="{left + chart_width + 60}" y="{legend_y + 4}">{xml_escape(name)}</text>')

    parts.append(f'<text class="tick" x="{left + chart_width / 2}" y="{height - 20}" text-anchor="middle">slice index</text>')
    write_text(output_path, svg_root(width, height, "\n".join(parts)))


def aggregate_batch_rows(dataset_summary_rows: list[dict[str, str]]) -> dict[str, list[dict[str, str]]]:
    grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in dataset_summary_rows:
        csv_path = row.get("batch_summary_csv", "").strip()
        if not csv_path:
            continue
        path = Path(csv_path)
        if not path.exists():
            continue
        for batch_row in read_csv(path):
            enriched = dict(batch_row)
            enriched["dataset_kind"] = row.get("dataset_kind", "unknown")
            enriched["case_id"] = row.get("case_id", "unknown")
            grouped[row.get("dataset_kind", "unknown")].append(enriched)
    return grouped


def create_figures(dataset_summary: Path, output_dir: Path) -> list[Path]:
    rows = read_csv(dataset_summary)
    output_dir.mkdir(parents=True, exist_ok=True)

    written: list[Path] = []
    status_path = output_dir / "dataset_case_status.svg"
    grouped_status_chart(rows, status_path)
    written.append(status_path)

    grouped_batch_rows = aggregate_batch_rows(rows)

    dice_labels = []
    dice_values = []
    infection_labels = []
    infection_values = []
    for dataset, batch_rows in sorted(grouped_batch_rows.items()):
        dice = [to_float(row.get("dice", "0")) for row in batch_rows if to_int(row.get("has_metrics", "0"))]
        infection = [to_float(row.get("infection_ratio", "0")) for row in batch_rows if to_int(row.get("has_infection", "0"))]
        if dice:
            dice_labels.append(dataset)
            dice_values.append(mean(dice))
        if infection:
            infection_labels.append(dataset)
            infection_values.append(mean(infection))

    if dice_values:
        dice_path = output_dir / "mean_dice_by_dataset.svg"
        bar_chart("Mean Dice by Dataset", dice_labels, dice_values, dice_path, color="#1565c0")
        written.append(dice_path)

    if infection_values:
        infection_path = output_dir / "mean_infection_ratio_by_dataset.svg"
        bar_chart("Mean Infection Ratio by Dataset", infection_labels, infection_values, infection_path, color="#c62828")
        written.append(infection_path)

    covid_series: dict[str, list[tuple[int, float]]] = {}
    for row in rows:
        if row.get("dataset_kind") != "covid-19-ct" or row.get("status") != "ok":
            continue
        batch_csv = row.get("batch_summary_csv", "").strip()
        if not batch_csv or not Path(batch_csv).exists():
            continue
        points = []
        for batch_row in read_csv(Path(batch_csv)):
            if to_int(batch_row.get("has_infection", "0")):
                points.append((to_int(batch_row.get("slice", "0")), to_float(batch_row.get("infection_ratio", "0"))))
        if points:
            covid_series[row.get("case_id", f"case_{len(covid_series) + 1}")] = points
        if len(covid_series) >= 5:
            break

    if covid_series:
        curve_path = output_dir / "covid_infection_ratio_curves.svg"
        line_chart("COVID Infection Ratio Curves", covid_series, curve_path)
        written.append(curve_path)

    return written


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate report SVG figures from dataset batch CSV outputs.")
    parser.add_argument(
        "--dataset-summary",
        default="results/datasets/dataset_summary.csv",
        type=Path,
        help="Path to dataset_summary.csv generated by the application.",
    )
    parser.add_argument(
        "--output-dir",
        default="results/figures",
        type=Path,
        help="Directory for generated SVG figures.",
    )
    args = parser.parse_args()

    written = create_figures(args.dataset_summary, args.output_dir)
    print("Generated figures:")
    for path in written:
        print(f"  {path}")
    if not written:
        print("  (none)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
